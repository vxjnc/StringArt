#pragma once

#include <vector>
#include <memory>
#include <random>

#include "src/image/Image.h"
#include "src/geometry/Point2.h"
#include "src/compute/OpenCLManager.h"

class StringArtGenerator
{
public:
    StringArtGenerator(const Image &input,
                       const int numNails,
                       const int maxConnections,
                       const std::vector<Color> &threadColors,
                       const float kDensity,
                       const bool isApplySobel,
                       std::mt19937 &gen);

    void generate(const float alpha);

    Image getResultImage();
    const std::vector<std::pair<Color, uint32_t>> &getSequence() const;

    std::vector<std::pair<Color, uint32_t>> loadSequence(const std::string_view filename);
    const Image &rebuildFromSequence(const std::vector<std::pair<Color, uint32_t>> &sequences, const float alpha);

private:
    void initializeNails(const int numNails, const short w, const short h);

    Image targetImage;
    Image currentImage;
    std::vector<std::pair<Color, uint32_t>> sequence;
    std::vector<uint16_t> density;
    std::vector<Point2s> nails;
    std::vector<Thread> threads;
    size_t maxIter;
    float kDensity;
    std::unique_ptr<OpenCLManager> ocl;
};
