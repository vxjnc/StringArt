#pragma once
#include <cstdint>

constexpr static uint8_t lerp_fixed(const uint8_t a, const uint8_t b, const uint16_t factor_t) noexcept
{
    constexpr int FIXED_POINT_SHIFT = 16;
    constexpr uint32_t FIXED_POINT_BASE = 1 << FIXED_POINT_SHIFT;
    constexpr uint32_t ROUNDING_ADDEND = 1 << (FIXED_POINT_SHIFT - 1);

    const uint32_t result = a * factor_t + b * (FIXED_POINT_BASE - factor_t);
    return static_cast<uint8_t>((result + ROUNDING_ADDEND) >> FIXED_POINT_SHIFT);
}