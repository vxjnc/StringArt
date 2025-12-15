#pragma once

#include <vector>
#include <memory>
#include <random>

#include "src/image/Image.h"
#include "src/geometry/Point2.h"
#include "src/geometry/Line.h"
#include "src/string_art/Therad.h"

class StringArtGenerator
{
private:
    Image originalImage;
    Image currentImage;
    std::vector<uint16_t> density;
    std::vector<Point2s> nails;
    std::vector<std::unique_ptr<Thread>> threads;
    std::vector<std::vector<std::unique_ptr<Line>>> lineCache;
    size_t maxIter;
    float kDensity;

    void precomputeAllLines();

    void initializeNails(const int numNails, const short w, const short h);

public:
    StringArtGenerator(const Image &input,
                       const int numNails,
                       const int maxConnections,
                       const std::vector<Color> &threadColors,
                       const float kDensity,
                       const bool isApplySobel,
                       std::mt19937 &gen);

    std::pair<Image, std::vector<std::pair<Color, uint32_t>>> generate(const float alpha);

    std::vector<std::pair<Color, uint32_t>> loadSequence(const std::string_view filename);

    const Image &rebuildFromSequence(const std::vector<std::pair<Color, uint32_t>> &sequences, const float alpha);
};
