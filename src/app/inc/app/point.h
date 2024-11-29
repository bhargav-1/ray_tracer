#pragma once
#include "tuple.h"

namespace ray_tracer {
class Point : public Tuple {
 public:
  Point() {
    x_ = 0;
    y_ = 0;
    z_ = 0;
  }
  constexpr Point(double x, double y, double z) : Tuple(x, y, z, 1) {}
  constexpr Point(const Tuple& tuple)  // NOLINT             // cppcheck-suppress [noExplicitConstructor]
      : Tuple(tuple.GetX(), tuple.GetY(), tuple.GetZ(), 1.0) {}  // NOLINT
};

}  // namespace ray_tracer
