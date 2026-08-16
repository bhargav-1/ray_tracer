#pragma once
#include <cmath>
#include <vector>
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "shape.h"
#include "tuple.h"

namespace ray_tracer {
class Intersection;
class Cylinder : public Shape {
 public:
  Cylinder() = delete;
  constexpr Cylinder(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Cylinder() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  auto static GlassCylinder(size_t index, double transparency = 1, double refractive_index = 1.5) -> Cylinder;
};

inline auto operator==(const Cylinder& lhs, const Cylinder& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Cylinder::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  // following is just the algorithm for ray/Cylinder intersection
  auto a_var = transformed_ray.GetDirection().GetX() * transformed_ray.GetDirection().GetX() +
           transformed_ray.GetDirection().GetZ() * transformed_ray.GetDirection().GetZ();
  // ray is parallel to y axis
  if (std::abs(a_var) < utilities::kEpsilon) {
    return {};
  }
  auto b_var = 2 * (transformed_ray.GetOrigin().GetX() * transformed_ray.GetDirection().GetX() +
                    transformed_ray.GetOrigin().GetZ() * transformed_ray.GetDirection().GetZ());
  auto c_var = transformed_ray.GetOrigin().GetX() * transformed_ray.GetOrigin().GetX() +
               transformed_ray.GetOrigin().GetZ() * transformed_ray.GetOrigin().GetZ() - 1;
  auto discriminant = (b_var * b_var) - (4 * a_var * c_var);
  if (discriminant < 0) {
    return {};
  }
  // this t is time/unit in the ray
  auto t1_var = (-b_var - std::sqrt(discriminant)) / (2 * a_var);
  auto t2_var = (-b_var + std::sqrt(discriminant)) / (2 * a_var);
  return {Intersection(t1_var, *this), Intersection(t2_var, *this)};
}
inline auto Cylinder::LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 {
  // all centers are at 0,0,0
  return {point.GetX(), 0, point.GetZ()};
}

inline auto Cylinder::GlassCylinder(const size_t index, const double transparency, const double refractive_index)
    -> Cylinder {
  auto cylinder = Cylinder(index);
  auto material = cylinder.GetMaterial();
  material.SetTransparency(transparency);
  material.SetRefractiveIndex(refractive_index);
  cylinder.SetMaterial(material);
  return cylinder;
}

}  // namespace ray_tracer
