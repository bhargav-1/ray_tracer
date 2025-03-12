#pragma once
#include <optional>
#include <vector>
#include "app/ray.h"
#include "app/sphere.h"

namespace ray_tracer {
class Intersection {
 public:
  Intersection(double time, Sphere sphere) : time_(time), sphere_(sphere){};
  auto friend operator==(Intersection const& lhs, Intersection const& rhs) -> bool;
  [[nodiscard]] auto GetTime() const { return time_; }
  [[nodiscard]] auto GetObject() const { return sphere_; }

 private:
  double time_;
  Sphere sphere_;
};

inline auto operator==(Intersection const& lhs, Intersection const& rhs) -> bool {
  return lhs.time_ == rhs.time_ && lhs.sphere_ == rhs.sphere_;
}

// can be constexpr in later cpp23++or cpp20++
//en.wikipedia.org/wiki/Line–sphere_intersection
inline auto Intersect(const Ray& ray, const Sphere& sphere) -> std::vector<Intersection> {
  // transform the ray to the sphere's coordinate system
  auto transform = sphere.GetTransform().Inverse();
  auto transformed_ray = ray.Transform(transform);

  // following is just the algorithm for ray/spher intersection
  const ray_tracer::Vec3 sphere_to_ray = transformed_ray.GetOrigin() - Point(0, 0, 0);
  const auto a_var = transformed_ray.GetDirection().Dot(transformed_ray.GetDirection());
  const auto b_var = 2 * transformed_ray.GetDirection().Dot(sphere_to_ray);
  const auto c_var = sphere_to_ray.Dot(sphere_to_ray) - 1;
  auto discriminant = (b_var * b_var) - (4 * a_var * c_var);
  if (discriminant < 0) {
    return {};
  }
  // this t is time/unit in the ray
  auto t1_var = (-b_var - std::sqrt(discriminant)) / (2 * a_var);
  auto t2_var = (-b_var + std::sqrt(discriminant)) / (2 * a_var);
  return {Intersection(t1_var, sphere), Intersection(t2_var, sphere)};
}

inline auto Hit(const std::vector<Intersection>& intersections) -> std::optional<Intersection> {
  auto current_intersection = std::optional<Intersection>();
  for (const auto& intersection : intersections) {
    if (intersection.GetTime() >= 0) {
      if (!current_intersection.has_value()) {
        current_intersection = std::optional<Intersection>(intersection);
      } else {
        if (intersection.GetTime() < current_intersection.value().GetTime()) {
          current_intersection = intersection;
        }
      }
    }
  }
  return current_intersection;
}

}  // namespace ray_tracer