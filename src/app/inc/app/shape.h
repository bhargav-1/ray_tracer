#pragma once
#include <optional>
#include <vector>
#include "app/bounding_box.h"
#include "intersection.h"
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "tuple.h"

namespace ray_tracer {
class Group;

class Shape {
 public:
  Shape() = delete;
  constexpr explicit Shape(std::size_t id) : id_(id) {}  // NOLINT [readability-identifier-naming]
  virtual ~Shape() = default;
  [[nodiscard]] constexpr auto GetId() const noexcept -> std::size_t { return id_; }
  friend auto operator==(const Shape& lhs, const Shape& rhs) -> bool;
  [[nodiscard]] auto GetTransform() const -> const Matrix<4, 4>& { return transform_; }
  [[nodiscard]] auto GetInverseTransform() const -> const Matrix<4, 4>& { return inverse_transform_; }
  void SetTransform(const Matrix<4, 4>& transform) {
    transform_ = transform;
    inverse_transform_ = transform.Inverse();
    transpose_inverse_ = Transpose(inverse_transform_);
  }
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
  [[nodiscard]] auto GetCastShadow() const -> bool { return cast_shadow_; }
  void SetCastShadow(const bool cast_shadow) { cast_shadow_ = cast_shadow; }
  void SetParentGroup(const std::weak_ptr<Shape>& parent_group) noexcept { parent_group_ = parent_group; }
  auto GetParentGroup() const -> std::weak_ptr<Shape> { return parent_group_; }
  auto WorldToObject(Point point) const -> Point {  // NOLINT (misc-no-recursion)
    if (const auto parent = parent_group_.lock()) {
      point = parent->WorldToObject(point);
    }
    return inverse_transform_ * point;
  }
  auto NormalToWorld(Vec3 normal) const -> Vec3 {  // NOLINT (misc-no-recursion)
    auto world_normal_vec = Vec3(transpose_inverse_ * normal);
    auto world_normal = world_normal_vec.Normalize();
    if (const auto parent = parent_group_.lock()) {
      world_normal = parent->NormalToWorld(world_normal);
    }
    return world_normal;
  }
  [[nodiscard]] auto NormalAt(const ray_tracer::Point& point) const -> Vec3 {
    const auto local_point = WorldToObject(point);
    auto const local_normal = LocalNormalAt(local_point);
    // https:  //www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/transforming-normals.html
    return NormalToWorld(local_normal).Normalize();
  }

  [[nodiscard]] auto ParentSpaceBounds() const -> BoundingBox { return BoundsOf().Transform(GetTransform()); }
  [[nodiscard]] virtual auto BoundsOf() const -> BoundingBox = 0;
  virtual void Divide(const int& threshold) {(void)threshold;}
 private:
  std::size_t id_;
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
  Matrix<4, 4> inverse_transform_ = Matrix<4, 4>::IdentityMatrix();
  Matrix<4, 4> transpose_inverse_ = Matrix<4, 4>::IdentityMatrix();
  Material material_;
  mutable std::optional<Ray> saved_ray_;
  bool cast_shadow_ = true;
  std::weak_ptr<Shape> parent_group_;
};

inline auto operator==(const Shape& lhs, const Shape& rhs) -> bool { return lhs.id_ == rhs.id_; }
}  // namespace ray_tracer
