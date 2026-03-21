#pragma once
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "tuple.h"
#include "intersection.h"
#include <vector>
#include <optional>

namespace ray_tracer {

class Shape {
public:
  Shape() = delete;
  constexpr explicit Shape(std::size_t id) : id_(id) {} // NOLINT [readability-identifier-naming]
  virtual ~Shape() = default;
  [[nodiscard]] constexpr auto GetId() const noexcept -> std::size_t { return id_; }
  friend auto operator==(const Shape& lhs, const Shape& rhs) -> bool;
  [[nodiscard]] auto GetTransform() const -> const Matrix<4, 4>& { return transform_; }
  [[nodiscard]] auto GetInverseTransform() const -> const Matrix<4, 4>& { return inverse_transform_; }
  void SetTransform(const Matrix<4, 4>& transform) {
    transform_ = transform;
    inverse_transform_ = transform.Inverse();
  }
  [[nodiscard]] auto NormalAt(const ray_tracer::Point& point) const -> Vec3 {
    const auto object_point = inverse_transform_ * point;
    auto const object_normal = LocalNormalAt(object_point);
    // https:  //www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/transforming-normals.html
    Vec3 const world_normal = Transpose(inverse_transform_) * object_normal;
    return world_normal.Normalize();
  };
  [[nodiscard]] auto GetMaterial() const -> const Material& { return material_; }
  void SetMaterial(const Material& material) { material_ = material; }
  [[nodiscard]] auto Intersect(const Ray& ray) const -> std::vector<Intersection> {
    const auto local_ray = ray.Transform(inverse_transform_);
    saved_ray_ = local_ray;
    return LocalIntersect(local_ray);
  };
  [[nodiscard]] auto GetSavedRay() const -> std::optional<Ray> { return saved_ray_; }
  [[nodiscard]] virtual auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> = 0;
  [[nodiscard]] virtual auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 = 0;

private:
  std::size_t id_;
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
  Matrix<4, 4> inverse_transform_ = Matrix<4, 4>::IdentityMatrix();
  Material material_;
  mutable std::optional<Ray> saved_ray_;
};

inline auto operator==(const Shape& lhs, const Shape& rhs) -> bool { return lhs.id_ == rhs.id_; }
}  // namespace ray_tracer
