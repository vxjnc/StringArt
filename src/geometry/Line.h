#pragma once

#include <vector>
#include "src/geometry/Point2.h"

class Line
{
private:
    std::vector<Point2s> pixels;

public:
    Line(Point2s start, Point2s end);
    const std::vector<Point2s> &getPixels() const noexcept;
};
