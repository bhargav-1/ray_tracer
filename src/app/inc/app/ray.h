#pragma once
#include "matrix.h"
#include "tuple.h"
#include "point.h"
#include "vec3.h"

namespace ray_tracer {
class Ray {
 public:
  constexpr Ray(Point origin, Vec3 direction) : origin_(std::move(origin)), direction_(std::move(direction)){};
  [[nodiscard]] constexpr auto GetOrigin() const -> Point { return origin_; }
  [[nodiscard]] constexpr auto GetDirection() const -> Vec3 { return direction_; }
  [[nodiscard]] constexpr auto Position(double time) const -> Point { return origin_ + direction_ * time; }


  private:
   Point origin_;
   Vec3 direction_;
};
}  // namespace ray_tracer
