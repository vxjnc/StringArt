#pragma once
#include <cstdint>
#include <algorithm>
#include <cmath>

template <typename T>
constexpr static T lerp(const T a, const T b, const float t) noexcept
{
    return static_cast<T>(a * t + b * (1.f - t));
}
