#pragma once
#include <cmath>
#include <limits>
#include <vector>
#include "app/materials/material.h"
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/tuple.h"
#include "app/rendering/ray.h"
#include "app/shapes/shape.h"

namespace ray_tracer {
class Intersection;
class Cone : public Shape {
 public:
  Cone() = delete;
  constexpr Cone(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Cone() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  [[nodiscard]] auto GetMinimum() const -> double { return minimum_; }
  [[nodiscard]] auto GetMaximum() const -> double { return maximum_; }
  void SetMinimum(const double minimum) {
    minimum_ = minimum;
    InvalidateBounds();
  }
  void SetMaximum(const double maximum) {
    maximum_ = maximum;
    InvalidateBounds();
  }
  void SetClosed(const bool closed) { closed_ = closed; }
  [[nodiscard]] auto IsClosed() const -> bool { return closed_; }
  [[nodiscard]] auto BoundsOf() const -> BoundingBox override;

 private:
  static auto CheckCap(const Ray& ray, double time, double cap) -> bool;
  void GetCapIntersection(const Ray& ray, std::vector<Intersection>& intersections) const;
  double minimum_ = -std::numeric_limits<double>::infinity();
  double maximum_ = std::numeric_limits<double>::infinity();
  bool closed_ = false;
};

inline auto operator==(const Cone& lhs, const Cone& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Cone::CheckCap(const Ray& ray, const double time, double cap) -> bool {
  auto x_var = ray.GetOrigin().GetX() + time * ray.GetDirection().GetX();
  auto z_var = ray.GetOrigin().GetZ() + time * ray.GetDirection().GetZ();
  return (x_var * x_var + z_var * z_var <= cap * cap);
}

inline void Cone::GetCapIntersection(const Ray& ray, std::vector<Intersection>& intersections) const {
  if (!closed_ || utilities::Equal(ray.GetDirection().GetY(), 0)) {
    return;
  }

  auto t_var = (minimum_ - ray.GetOrigin().GetY()) / ray.GetDirection().GetY();
  if (CheckCap(ray, t_var, minimum_)) {
    intersections.emplace_back(t_var, *this);
  }

  auto t_var2 = (maximum_ - ray.GetOrigin().GetY()) / ray.GetDirection().GetY();
  if (CheckCap(ray, t_var2, maximum_)) {
    intersections.emplace_back(t_var2, *this);
  }
}
inline auto Cone::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  // following is just the algorithm for ray/Cone intersection
  std::vector<Intersection> intersections;
  auto a_var = transformed_ray.GetDirection().GetX() * transformed_ray.GetDirection().GetX() +
               transformed_ray.GetDirection().GetZ() * transformed_ray.GetDirection().GetZ() -
               transformed_ray.GetDirection().GetY() * transformed_ray.GetDirection().GetY();
  auto b_var = 2 * (transformed_ray.GetOrigin().GetX() * transformed_ray.GetDirection().GetX() +
                    transformed_ray.GetOrigin().GetZ() * transformed_ray.GetDirection().GetZ() -
                    transformed_ray.GetOrigin().GetY() * transformed_ray.GetDirection().GetY());
  auto c_var = transformed_ray.GetOrigin().GetX() * transformed_ray.GetOrigin().GetX() +
               transformed_ray.GetOrigin().GetZ() * transformed_ray.GetOrigin().GetZ() -
               transformed_ray.GetOrigin().GetY() * transformed_ray.GetOrigin().GetY();
  auto discriminant = (b_var * b_var) - (4 * a_var * c_var);

  if (utilities::Equal(a_var, 0)) {
    if (!utilities::Equal(b_var, 0)) {
      intersections.emplace_back(-c_var / b_var, *this);
    }
    GetCapIntersection(transformed_ray, intersections);
    return intersections;
  }

  if (discriminant >= 0) {
    // this t is time/unit in the ray
    auto t1_var = (-b_var - std::sqrt(discriminant)) / (2 * a_var);
    auto t2_var = (-b_var + std::sqrt(discriminant)) / (2 * a_var);
    if (t1_var > t2_var) {
      std::swap(t1_var, t2_var);
    }
    auto y0_var = transformed_ray.GetOrigin().GetY() + t1_var * transformed_ray.GetDirection().GetY();
    if (y0_var > minimum_ && y0_var < maximum_) {
      intersections.emplace_back(t1_var, *this);
    }
    auto y1_var = transformed_ray.GetOrigin().GetY() + t2_var * transformed_ray.GetDirection().GetY();
    if (y1_var > minimum_ && y1_var < maximum_) {
      intersections.emplace_back(t2_var, *this);
    }
  }
  GetCapIntersection(transformed_ray, intersections);
  return intersections;
}
inline auto Cone::LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 {
  auto dist = point.GetX() * point.GetX() + point.GetZ() * point.GetZ();
  Vec3 result;
  auto y = std::sqrt(dist);  // NOLINT
  if (point.GetY() > 0) {
    y = -y;
  }
  if (dist < 1 && point.GetY() >= maximum_ - utilities::kEpsilon) {
    result = {0, y, 0};
  } else if (dist < 1 && point.GetY() <= minimum_ + utilities::kEpsilon) {
    result = {0, -y, 0};
  } else {
    result = {point.GetX(), y, point.GetZ()};
  }
  return result;
}

inline auto Cone::BoundsOf() const -> BoundingBox {
  auto limit = std::max(std::abs(minimum_), std::abs(maximum_));
  return {Point(-limit, minimum_, -limit), Point(limit, maximum_, limit)};
}
}  // namespace ray_tracer
