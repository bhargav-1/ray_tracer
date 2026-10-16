#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
#include "app/materials/material.h"
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/rendering/ray.h"
#include "app/shapes/shape.h"
#include "app/primitives/tuple.h"

namespace ray_tracer {
class Intersection;
class Cube : public Shape {
 public:
  Cube() = delete;
  constexpr Cube(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Cube() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  [[nodiscard]] auto BoundsOf() const -> BoundingBox override;
};

inline auto operator==(const Cube& lhs, const Cube& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Cube::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  double tmin = 0;
  double tmax = 0;
  if (!BoundsOf().Intersects(transformed_ray, tmin, tmax)) {
    return {};
  }
  return {Intersection(tmin, *this), Intersection(tmax, *this)};
}

inline auto Cube::LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 {
  auto maxc = std::max(std::max(std::abs(point.GetX()), std::abs(point.GetY())), std::abs(point.GetZ()));
  auto result = Vec3(0, 0, 0);
  if (maxc == std::abs(point.GetX())) {
    result = {point.GetX(), 0, 0};
  } else if (maxc == std::abs(point.GetY())) {
    result = {0, point.GetY(), 0};
  } else {
    result = {0, 0, point.GetZ()};
  }
  return result;
}
inline auto Cube::BoundsOf() const -> BoundingBox { return {Point(-1, -1, -1), Point(1, 1, 1)}; }
}  // namespace ray_tracer
