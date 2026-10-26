#pragma once
#include <cmath>
#include "app/primitives/tuple.h"

namespace ray_tracer {
class Vec3 : public Tuple {
 public:
  constexpr Vec3() noexcept = default;
  constexpr Vec3(double x_coordinate, double y_coordinate, double z_coordinate)
      : Tuple(x_coordinate, y_coordinate, z_coordinate, 0) {}
  constexpr Vec3(const Tuple& tuple) : Tuple(tuple) {}  // NOLINT // cppcheck-suppress [noExplicitConstructor]
  [[nodiscard]] constexpr auto Magnitude() const -> double { return std::sqrt((x_ * x_) + (y_ * y_) + (z_ * z_)); }
  [[nodiscard]] constexpr auto Normalize() const -> Vec3 {
    const auto mag = Magnitude();
    return Vec3{x_ / mag, y_ / mag, z_ / mag};
  }
  [[nodiscard]] constexpr auto Dot(const Vec3& vec) const -> double {
    return (x_ * vec.x_) + (y_ * vec.y_) + (z_ * vec.z_);
  }
  [[nodiscard]] constexpr auto Cross(const Vec3& vec) const -> Vec3 {
    return Vec3{(y_ * vec.z_) - (z_ * vec.y_), (z_ * vec.x_) - (x_ * vec.z_), (x_ * vec.y_) - (y_ * vec.x_)};
  }
  [[nodiscard]] constexpr auto Reflect(const Vec3& normal) const -> Vec3 { return {*this - normal * 2 * Dot(normal)}; }
};

}  // namespace ray_tracer
