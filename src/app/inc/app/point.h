#pragma once
#include "tuple.h"

namespace ray_tracer {
class Point : public Tuple {
 public:
  Point() = default;
  Point(double x, double y, double z) : Tuple(x, y, z, 1) {}
//  explicit Point(const Tuple& tuple) : Tuple(tuple) {};
  Point(const Tuple& tuple) : Tuple(tuple.GetX(), tuple.GetY(), tuple.GetZ(), 1.0) {}
};

}  // namespace ray_tracer