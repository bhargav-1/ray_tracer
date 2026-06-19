#pragma once
#include <algorithm>
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
class Cube : public Shape {
 public:
  Cube() = delete;
  constexpr Cube(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Cube() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;

 private:
  [[nodiscard]] static auto CheckAxis(double origin, double direction) -> std::pair<double, double>;
};

inline auto operator==(const Cube& lhs, const Cube& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Cube::CheckAxis(double origin, double direction) -> std::pair<double, double> {
  auto tmin_numerator = -1 - origin;
  auto tmax_numerator = 1 - origin;
  double tmin = 0;
  double tmax = 0;
  if (std::abs(direction) >= utilities::kEpsilon) {
    tmin = tmin_numerator / direction;
    tmax = tmax_numerator / direction;
  } else {
    tmin = tmin_numerator * std::numeric_limits<double>::infinity();
    tmax = tmax_numerator * std::numeric_limits<double>::infinity();
  }
  if (tmin > tmax) {
    std::swap(tmin, tmax);
  }
  return {tmin, tmax};
}
inline auto Cube::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  auto x_axis = CheckAxis(transformed_ray.GetOrigin().GetX(), transformed_ray.GetDirection().GetX());
  auto y_axis = CheckAxis(transformed_ray.GetOrigin().GetY(), transformed_ray.GetDirection().GetY());
  auto z_axis = CheckAxis(transformed_ray.GetOrigin().GetZ(), transformed_ray.GetDirection().GetZ());
  auto tmin = std::max(std::max(x_axis.first, y_axis.first), z_axis.first);
  auto tmax = std::min(std::min(x_axis.second, y_axis.second), z_axis.second);
  if (tmin > tmax) {
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
}  // namespace ray_tracer
