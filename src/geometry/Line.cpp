#include "Line.h"

#include <cmath>

Line::Line(Point2s start, Point2s end)
{
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;

    if (dx >= dy)
    {
        int err = 2 * dy - dx;
        int y = start.y;
        pixels.reserve(dx + 1);

        for (int x = start.x; x != end.x + sx; x += sx)
        {
            pixels.emplace_back(x, y);
            if (err >= 0)
            {
                y += sy;
                err -= 2 * dx;
            }
            err += 2 * dy;
        }
    }
    else
    {
        int err = 2 * dx - dy;
        int x = start.x;
        pixels.reserve(dy + 1);

        for (int y = start.y; y != end.y + sy; y += sy)
        {
            pixels.emplace_back(x, y);
            if (err >= 0)
            {
                x += sx;
                err -= 2 * dy;
            }
            err += 2 * dx;
        }
    }
}
const std::vector<Point2s> &Line::getPixels() const noexcept { return pixels; }
