#include "StringArt.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>

void StringArtGenerator::initializeNails(const int numNails, const short w, const short h)
{
    const short radiusW = w / 2 - 1;
    const short radiusH = h / 2 - 1;
    const Point2s center(static_cast<short>(w / 2), static_cast<short>(h / 2));
    nails.reserve(numNails);
    for (int i = 0; i < numNails; ++i)
    {
        const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(numNails);
        const short x = static_cast<short>(center.x + radiusW * std::cos(angle));
        const short y = static_cast<short>(center.y + radiusH * std::sin(angle));
        nails.emplace_back(std::clamp<short>(x, 0, w - 1),
                           std::clamp<short>(y, 0, h - 1));
    }
}

StringArtGenerator::StringArtGenerator(const Image &input,
                                       const int numNails,
                                       const int maxConnections,
                                       const std::vector<Color> &threadColors,
                                       const float kDensity,
                                       const bool isApplySobel,
                                       std::mt19937 &gen)
    : maxIter(maxConnections), kDensity(kDensity)
{
    targetImage = input;
    if (isApplySobel)
    {
        Image edges(targetImage.calculateEdges());
        for (int i = 0; i < edges.height() * edges.width(); ++i)
        {
            for (int j = 0; j < targetImage.channels(); ++j)
                targetImage[i * targetImage.channels()][j] &= edges[i][j];
        }
    }

    targetImage.resize(512, 512);

    currentImage = Image(targetImage.width(), targetImage.height(), targetImage.channels());

    density.resize(currentImage.width() * currentImage.height(), 0);

    initializeNails(numNails, currentImage.width(), currentImage.height());

    std::uniform_int_distribution<> dis(0, numNails - 1);
    for (auto &&color : threadColors)
        threads.emplace_back(color, dis(gen));

    ocl = std::make_unique<OpenCLManager>(512, 512, numNails);
}

void StringArtGenerator::generate(const float alpha)
{
    ocl->setupResources(targetImage, currentImage, density, nails, threads.size(), maxIter);
    ocl->loadProgram("kernels/kernel.cl");
    ocl->updateThreads(threads);
    ocl->setupArgs(alpha, kDensity);

    for (size_t i = 0; i < maxIter; ++i)
    {
        ocl->runScores();
        ocl->runMinReduction();
        ocl->runDraw(i);
    }
    ocl->finish();

    ocl->downloadResult(currentImage);

    std::vector<uint32_t> rawSeq = ocl->downloadSequence(maxIter);
    std::vector<std::pair<Color, uint32_t>> sequence;
    sequence.reserve(maxIter);
    for (size_t i = 0; i < maxIter; ++i)
    {
        uint32_t tIdx = rawSeq[i * 2];
        uint32_t nIdx = rawSeq[i * 2 + 1];
        sequence.emplace_back(threads[tIdx].color, nIdx);
    }
}

Image StringArtGenerator::getResultImage()
{
    Image out(targetImage.width(), targetImage.height(), 4);
    ocl->downloadResult(out);
    return out;
}

const std::vector<std::pair<Color, uint32_t>> &StringArtGenerator::getSequence() const { return sequence; }

std::vector<std::pair<Color, uint32_t>> StringArtGenerator::loadSequence(const std::string_view filename)
{
    std::ifstream fin(filename.data());
    std::vector<std::pair<Color, uint32_t>> sequences;

    sequences.reserve(std::count(std::istreambuf_iterator<char>(fin),
                                 std::istreambuf_iterator<char>(), '\n'));
    fin.clear();
    fin.seekg(0);

    while (!fin.eof())
    {
        short r, g, b, nailInd;
        fin >> r >> g >> b >> nailInd;

        sequences.emplace_back(Color{{static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)}}, nailInd);
    }

    return sequences;
}

const Image &StringArtGenerator::rebuildFromSequence(const std::vector<std::pair<Color, uint32_t>> &sequences, const float alpha)
{
    std::unordered_map<Color, int> colorToCurrentNail;
    for (const auto &entry : sequences)
    {
        const Color &color = entry.first;
        const int currentNail = entry.second;

        const auto it = colorToCurrentNail.find(color);
        if (it != colorToCurrentNail.end())
        {
            const int prevNail = it->second;
            const int a = std::min(prevNail, currentNail);
            const int b = std::max(prevNail, currentNail);

            // const Line &line = *lineCache[a][b]; TODO сделать через OpenCL
            // for (const auto &p : line.getPixels())
            // {
            //     uint8_t *pixel = currentImage(p.x, p.y);
            //     for (int i = 0; i < 3; ++i)
            //         pixel[i] = lerp(color[i], pixel[i], alpha);
            // }
        }

        colorToCurrentNail[color] = currentNail;
    }

    return currentImage;
}
