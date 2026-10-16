#pragma once
#include "app/primitives/tuple.h"

namespace ray_tracer {
class Point : public Tuple {
 public:
  constexpr Point() noexcept = default;
  constexpr Point(double x_coordinate, double y_coordinate, double z_coordinate)
      : Tuple(x_coordinate, y_coordinate, z_coordinate, 1) {}
  constexpr Point(const Tuple& tuple)  // NOLINT             // cppcheck-suppress [noExplicitConstructor]
      : Tuple(tuple.GetX(), tuple.GetY(), tuple.GetZ(), 1.0) {}  // NOLINT
};

}  // namespace ray_tracer
