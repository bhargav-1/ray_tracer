#pragma once
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "shape.h"
#include <cmath>
#include <vector>
#include "tuple.h"

namespace ray_tracer {
class Intersection;
class Sphere : public Shape{
 public:
  Sphere() = delete;
  constexpr Sphere(size_t id) : Shape(id)  {} // NOLINT [readability-identifier-naming]
  ~Sphere() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override;
};

inline auto operator==(const Sphere& lhs, const Sphere& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Sphere::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {

    // following is just the algorithm for ray/sphere intersection
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
    return {Intersection(t1_var,*this), Intersection(t2_var, *this)};
}
}  // namespace ray_tracer
