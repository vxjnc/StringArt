#include <CL/opencl.hpp>
#include <vector>
#include <string_view>
#include "src/image/Image.h"
#include "src/utils/color_utils.h"
#include "src/geometry/Point2.h"
#include <fstream>
#include <span>
#include <iostream>
#include <tuple>

struct Thread
{
    Color color;
    uint32_t currentNail;
};

struct alignas(16) GpuThread
{
    cl_uchar4 color;
    cl_uint currentNail;
    uint32_t padding[2];

    GpuThread(Color c, uint32_t nail)
        : color{c[0], c[1], c[2], 255}, currentNail(nail) {}
};

class OpenCLManager
{
private:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;

    struct
    {
        cl::Kernel scores;
        cl::Kernel draw;
        cl::Kernel findMin;
    } kernels;

    struct
    {
        cl::Buffer imgOriginal;
        cl::Buffer imgCurrent;
        cl::Buffer nails;
        cl::Buffer density;
        cl::Buffer scores;
        cl::Buffer threads;
        cl::Buffer bestResult;
        cl::Buffer sequence;
    } buffers;

    uint32_t width, height, numNails;
    int threadsCount = 0;
    std::vector<GpuThread> hostThreads;

    size_t maxWorkGroupSize;

public:
    OpenCLManager(uint32_t w, uint32_t h, uint32_t nailsCount)
        : width(w), height(h), numNails(nailsCount)
    {
        initDevice();
    }

    void initDevice()
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.empty())
            throw std::runtime_error("No OpenCL platforms found");

        std::vector<cl::Device> devices;
        platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
        if (devices.empty())
            throw std::runtime_error("No GPU devices found");

        context = cl::Context(devices[0]);
        queue = cl::CommandQueue(context, devices[0]);

        devices[0].getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &maxWorkGroupSize);
        if (maxWorkGroupSize > 1024)
            maxWorkGroupSize = 1024;
    }

    void setupResources(const Image &orig, const Image &current,
                        std::span<const uint16_t> density,
                        std::span<const Point2s> nails,
                        int threadsSize, int maxIter)
    {
        if (orig.channels() != 4 || current.channels() != 4)
            throw std::runtime_error("Images must be RGBA (4 channels)");

        threadsCount = threadsSize;
        hostThreads.reserve(threadsSize);

        auto createBuffer = [&](cl_mem_flags flags, size_t size, void *ptr = nullptr)
        {
            return cl::Buffer(context, flags | (ptr ? CL_MEM_COPY_HOST_PTR : 0), size, ptr);
        };
        std::vector<cl_ushort2> nailsData;
        nailsData.reserve(nails.size());
        for (const auto &p : nails)
            nailsData.push_back({(cl_ushort)p.x, (cl_ushort)p.y});

        buffers.imgOriginal = createBuffer(CL_MEM_READ_ONLY, width * height * 4, (void *)orig.data());
        buffers.imgCurrent = createBuffer(CL_MEM_READ_WRITE, width * height * 4, (void *)current.data());
        buffers.density = createBuffer(CL_MEM_READ_WRITE, density.size_bytes(), (void *)density.data());
        buffers.scores = createBuffer(CL_MEM_READ_WRITE, numNails * threadsSize * sizeof(float));
        buffers.threads = createBuffer(CL_MEM_READ_WRITE, threadsSize * sizeof(GpuThread));
        buffers.bestResult = createBuffer(CL_MEM_READ_WRITE, 2 * sizeof(cl_uint));
        buffers.sequence = createBuffer(CL_MEM_READ_WRITE, maxIter * 2 * sizeof(cl_uint));
        buffers.nails = createBuffer(CL_MEM_READ_ONLY, nailsData.size() * sizeof(cl_ushort2), nailsData.data());
    }

    void loadProgram(std::string_view filename)
    {
        std::ifstream file(filename.data());
        if (!file)
            throw std::runtime_error("Kernel file not found: " + std::string(filename));

        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        program = cl::Program(context, source);

        if (program.build({context.getInfo<CL_CONTEXT_DEVICES>()[0]}) != CL_SUCCESS)
        {
            throw std::runtime_error("OpenCL Build Error:\n" +
                                     program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>()[0]));
        }

        kernels.scores = cl::Kernel(program, "calculate_all_threads_scores");
        kernels.findMin = cl::Kernel(program, "find_min_reduction");
        kernels.draw = cl::Kernel(program, "draw_line");
    }

    void setupArgs(float alpha, float kDensity)
    {
        auto &ks = kernels.scores;
        ks.setArg(0, buffers.imgOriginal);
        ks.setArg(1, buffers.imgCurrent);
        ks.setArg(2, buffers.nails);
        ks.setArg(3, buffers.density);
        ks.setArg(4, buffers.scores);
        ks.setArg(5, buffers.threads);
        ks.setArg(6, (cl_uint)threadsCount);
        ks.setArg(7, (cl_uint)numNails);
        ks.setArg(8, (cl_uint)width);
        ks.setArg(9, alpha);
        ks.setArg(10, kDensity);

        auto &km = kernels.findMin;
        km.setArg(0, buffers.scores);
        km.setArg(1, buffers.bestResult);
        km.setArg(2, (cl_uint)(numNails * threadsCount));
        km.setArg(3, (cl_uint)numNails);
        km.setArg(4, cl::Local(maxWorkGroupSize * sizeof(float)));
        km.setArg(5, cl::Local(maxWorkGroupSize * sizeof(cl_uint)));

        auto &kd = kernels.draw;
        kd.setArg(0, buffers.imgCurrent);
        kd.setArg(1, buffers.density);
        kd.setArg(2, buffers.nails);
        kd.setArg(3, buffers.threads);
        kd.setArg(4, buffers.bestResult);
        kd.setArg(5, (cl_uint)width);
        kd.setArg(6, alpha);
        kd.setArg(7, buffers.sequence);
    }

    void runScores()
    {
        queue.enqueueNDRangeKernel(kernels.scores, cl::NullRange, cl::NDRange(numNails, threadsCount));
    }

    void runMinReduction()
    {
        queue.enqueueNDRangeKernel(kernels.findMin, cl::NullRange, cl::NDRange(maxWorkGroupSize), cl::NDRange(maxWorkGroupSize));
    }

    void runDraw(unsigned int iteration)
    {
        kernels.draw.setArg(8, static_cast<cl_uint>(iteration));
        queue.enqueueNDRangeKernel(kernels.draw, cl::NullRange, cl::NDRange(width));
    }

    void updateThreads(std::span<const Thread> threads)
    {
        hostThreads.clear();
        for (const auto &t : threads)
            hostThreads.emplace_back(t.color, t.currentNail);

        queue.enqueueWriteBuffer(buffers.threads, CL_TRUE, 0, hostThreads.size() * sizeof(GpuThread), hostThreads.data());
    }

    void downloadResult(Image &out)
    {
        queue.enqueueReadBuffer(buffers.imgCurrent, CL_TRUE, 0, width * height * 4, out.data());
    }

    void finish() { queue.finish(); }

    std::vector<uint32_t> downloadSequence(int maxIter)
    {
        std::vector<uint32_t> hostSeq(maxIter * 2);
        queue.enqueueReadBuffer(buffers.sequence, CL_TRUE, 0, hostSeq.size() * sizeof(uint32_t), hostSeq.data());
        return hostSeq;
    }
};