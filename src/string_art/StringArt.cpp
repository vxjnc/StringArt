#include "StringArt.h"
#include <cmath>
#include <fstream>
#include <numbers>

void StringArtGenerator::precomputeAllLines()
{
    const int numNails = nails.size();
    lineCache.resize(numNails);
    for (int i = 0; i < numNails; ++i)
    {
        lineCache[i].resize(i + 1);
        for (int j = 0; j < i + 1; ++j)
        {
            lineCache[i][j] = std::make_unique<Line>(nails[i], nails[j]);
        }
    }
}

void StringArtGenerator::initializeNails(const int numNails, const short w, const short h)
{
    const short radiusW = w / 2 - 1;
    const short radiusH = h / 2 - 1;
    const Point2s center(static_cast<short>(w / 2), static_cast<short>(h / 2));
    nails.reserve(numNails);
    for (int i = 0; i < numNails; ++i)
    {
        const float angle = 2.f * std::numbers::pi_v<float> * i / numNails;
        const short x = center.x + radiusW * std::cos(angle);
        const short y = center.y + radiusH * std::sin(angle);
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
    originalImage = input;

    if (isApplySobel)
    {
        Image edges(originalImage.calculateEdges());
        for (int i = 0; i < edges.height() * edges.width(); ++i)
        {
            for (int j = 0; j < 3; ++j)
                originalImage[i * 3][j] &= edges[i][j];
        }
    }

    originalImage.resize(512, 512);

    currentImage = Image(originalImage.width(), originalImage.height(), originalImage.channels());

    density.resize(currentImage.width() * currentImage.height(), 0);

    initializeNails(numNails, currentImage.width(), currentImage.height());
    precomputeAllLines();

    std::uniform_int_distribution<> dis(0, numNails - 1);
    for (auto &&color : threadColors)
        threads.emplace_back(std::make_unique<Thread>(dis(gen), color));
}

std::pair<Image, std::vector<std::pair<Color, uint32_t>>> StringArtGenerator::generate(const float alpha)
{
    std::vector<std::pair<Color, uint32_t>> sequence;
    sequence.reserve(maxIter);

    for (size_t renderIter = 0; renderIter < maxIter; ++renderIter)
    {
        float bestScore = std::numeric_limits<float>::max();
        Thread *bestThread = nullptr;

        for (auto &thread : threads)
        {
            const float score = thread->getNextNailWeight(nails, originalImage, currentImage, lineCache, alpha, density, kDensity);

            if (score < bestScore)
            {
                bestScore = score;
                bestThread = thread.get();
            }
        }

        if (bestThread)
        {
            sequence.emplace_back(bestThread->getColor(), bestThread->currentNail);

            bestThread->addLineToImage(currentImage, density, alpha);
            bestThread->moveToNextNail();
        }
    }

    return {std::move(currentImage), std::move(sequence)};
}

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
        int r, g, b, nailInd;
        fin >> r >> g >> b >> nailInd;

        sequences.emplace_back(Color{{(uint8_t)r, (uint8_t)g, (uint8_t)b}}, nailInd);
    }

    return sequences;
}

const Image &StringArtGenerator::rebuildFromSequence(const std::vector<std::pair<Color, uint32_t>> &sequences, const float alpha)
{
    std::unordered_map<Color, int> colorToCurrentNail;
    const uint16_t alpha_factor = static_cast<uint16_t>(std::lround(alpha * 65536.0f));
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

            const Line &line = *lineCache[a][b];
            for (const auto &p : line.getPixels())
            {
                uint8_t *pixel = currentImage(p.x, p.y);
                for (int i = 0; i < 3; ++i)
                    pixel[i] = lerp_fixed(color[i], pixel[i], alpha_factor);
            }
        }

        colorToCurrentNail[color] = currentNail;
    }

    return currentImage;
}
