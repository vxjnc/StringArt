#pragma once

#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <array>
#include <limits>
#include "src/geometry/Line.h"
#include "src/image/Image.h"
#include "src/utils/math_utils.h"
#include "src/utils/color_utils.h"

class Thread
{
public:
    unsigned int currentNail;
    Line *nextLine;

    Thread(const unsigned int startNail, const Color &col) noexcept
        : currentNail(startNail), nextLine(nullptr), color(col)
    {
    }

    float getNextNailWeight(const std::vector<Point2s> &nails,
                            const Image &original,
                            const Image &current,
                            const std::vector<std::vector<std::unique_ptr<Line>>> &lineCache,
                            const float alpha,
                            const std::vector<uint16_t> &density,
                            const float kDensity)
    {
        if (nextValid)
            return currentDist;

        float minDist = std::numeric_limits<float>::max();
        size_t minDistIndex = 0;
        Line *bestLine = nullptr;

        for (unsigned int i = 0; i < nails.size(); ++i)
        {
            if (i == currentNail)
                continue;

            const int a = std::max<int>(currentNail, i);
            const int b = std::min<int>(currentNail, i);

            const auto &lineUniquePtr = lineCache[a][b];
            const auto &line = *lineUniquePtr;
            const bool alreadyConnected = prevConnections[currentNail].size() > i &&
                                          prevConnections[currentNail][i];

            const float dist = alreadyConnected ? std::numeric_limits<float>::max()
                                                : calculateLineScore(line, original, current,
                                                                     density, alpha, kDensity);

            if (dist < minDist)
            {
                minDist = dist;
                minDistIndex = i;
                bestLine = lineUniquePtr.get();
            }
        }

        nextNail = static_cast<int>(minDistIndex);
        nextLine = bestLine;
        nextValid = true;
        currentDist = minDist;
        return minDist;
    }

    void moveToNextNail()
    {
        if (!nextValid)
            return;

        if (prevConnections[currentNail].size() <= static_cast<size_t>(nextNail))
            prevConnections[currentNail].resize(nextNail + 1, false);

        prevConnections[currentNail][nextNail] = true;
        currentNail = nextNail;
        nextValid = false;
    }

    void addLineToImage(Image &image, std::vector<uint16_t> &density, const float alpha)
    {
        if (!nextLine)
            return;

        for (const auto &p : nextLine->getPixels())
        {
            uint8_t *pixel = image(p.x, p.y);
            for (int i = 0; i < 3; ++i)
                pixel[i] = lerp<uint8_t>(color[i], pixel[i], alpha);

            density[p.y * image.width() + p.x] += 1;
        }
    }

    float calculateLineScore(const Line &line,
                             const Image &original,
                             const Image &current,
                             const std::vector<uint16_t> &density,
                             const float alpha,
                             const float kDensity) const
    {
        int totalDiff = 0;
        int totalDensity = 0;
        const auto &pixels = line.getPixels();

        const int cur_w = current.width();
        for (const auto &p : pixels)
        {
            const int shift = p.y * cur_w + p.x;
            const uint8_t *orig = original[shift * 3];
            const uint8_t *curr = current[shift * 3];

            for (int i = 0; i < 3; ++i)
            {
                const int16_t diffOB = orig[i] - lerp(color[i], curr[i], alpha);
                const int16_t diffOC = orig[i] - curr[i];
                totalDiff += std::min<int32_t>(0, (diffOB - diffOC) * (diffOB + diffOC));
            }

            totalDensity += density[shift];
        }

        return (totalDiff + kDensity * totalDensity) / static_cast<float>(pixels.size());
    }

    Color getColor() const noexcept { return color; }

private:
    Color color;
    float currentDist = std::numeric_limits<float>::infinity();
    int nextNail = -1;
    bool nextValid = false;
    std::map<int, std::vector<uint8_t>> prevConnections;
};
