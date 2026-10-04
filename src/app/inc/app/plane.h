#pragma once
#include <cmath>
#include <limits>
#include <vector>
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "shape.h"
#include "tuple.h"
#include "utilities.h"

namespace ray_tracer {
class Intersection;
class Plane : public Shape {
  // its xz plane by default
 public:
  Plane() = delete;
  constexpr Plane(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Plane() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  [[nodiscard]] auto BoundsOf() const -> BoundingBox override;
};

inline auto operator==(const Plane& lhs, const Plane& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Plane::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  if (std::abs(transformed_ray.GetDirection().GetY()) < utilities::kEpsilon) {
    return {};
  }
  // P(t) = origin + t * direction
  // Take only the y-component:
  // y(t) = origin.y + t * direction.y
  // For the xz-plane, intersection means:
  // y(t) = 0
  // So:
  // 0 = origin.y + t * direction.y
  // t * direction.y = -origin.y
  // t = -origin.y / direction.y

  return {Intersection(-transformed_ray.GetOrigin().GetY() / transformed_ray.GetDirection().GetY(), *this)};
}
inline auto Plane::LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 {
  // its y axis by default
  (void)point;
  return {0, 1, 0};
}
inline auto Plane::BoundsOf() const -> BoundingBox {
  return {
Point(-1 * std::numeric_limits<double>::infinity(), 0, -1 * std::numeric_limits<double>::infinity()),
Point(std::numeric_limits<double>::infinity(), 0, std::numeric_limits<double>::infinity())
  };
}
}  // namespace ray_tracer
