#pragma once
#include <complex>

namespace utilities {
constexpr double kEpsilon = 0.0001;
constexpr auto Equal(double left, double right) -> bool { return (std::abs(left - right) < kEpsilon); }
}  // namespace utilities