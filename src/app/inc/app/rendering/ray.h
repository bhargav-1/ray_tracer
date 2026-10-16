#pragma once
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/tuple.h"
#include "app/primitives/vec3.h"

namespace ray_tracer {
class Ray {
 public:
  constexpr Ray(Point origin, Vec3 direction) : origin_(std::move(origin)), direction_(std::move(direction)){};
  [[nodiscard]] constexpr auto GetOrigin() const -> const Point& { return origin_; }
  [[nodiscard]] constexpr auto GetDirection() const -> const Vec3& { return direction_; }
  [[nodiscard]] constexpr auto Position(double time) const -> Point { return origin_ + direction_ * time; }
  [[nodiscard]] constexpr auto Transform(const Matrix<4, 4>& matrix) const -> Ray {
    return Ray{matrix * origin_, matrix * direction_};
  }

 private:
  Point origin_;
  Vec3 direction_;
};
}  // namespace ray_tracer
