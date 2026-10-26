#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>
#include "app/materials/material.h"
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/tuple.h"
#include "app/primitives/utilities.h"
#include "app/rendering/bounding_box.h"
#include "app/rendering/intersection.h"
#include "app/rendering/ray.h"

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
    // Matrix equality elsewhere is approximate; this fast path must be exact.
    has_identity_transform_ = true;
    for (std::size_t row = 0; row < 4; ++row) {
      for (std::size_t column = 0; column < 4; ++column) {
        const double expected = row == column ? 1.0 : 0.0;
        if (transform_(row, column) != expected) {
          has_identity_transform_ = false;
        }
      }
    }
    InvalidateBounds();
  }
  [[nodiscard]] auto GetMaterial() const -> const Material& { return material_; }
  void SetMaterial(const Material& material) { material_ = material; }
  [[nodiscard]] auto Intersect(const Ray& ray) const -> std::vector<Intersection> {
    if (has_identity_transform_) {
      return LocalIntersect(ray);
    }
    const auto local_ray = ray.Transform(inverse_transform_);
    return LocalIntersect(local_ray);
  };
  [[nodiscard]] auto Occludes(const Ray& ray, const double max_distance) const -> bool {
    if (!(max_distance > utilities::kEpsilon)) {
      return false;
    }
    return has_identity_transform_ ? LocalOccludes(ray, max_distance)
                                   : LocalOccludes(ray.Transform(inverse_transform_), max_distance);
  }
  [[nodiscard]] virtual auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> = 0;
  [[nodiscard]] virtual auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 = 0;
  [[nodiscard]] virtual auto LocalNormalAt(const Point& point,
                                           [[maybe_unused]] const Intersection& intersection) const -> Vec3 {
    return LocalNormalAt(point);
  }
  [[nodiscard]] auto GetCastShadow() const -> bool { return cast_shadow_; }
  void SetCastShadow(const bool cast_shadow) { cast_shadow_ = cast_shadow; }
  void SetParentGroup(const std::weak_ptr<Shape>& parent_group) noexcept { parent_group_ = parent_group; }
  [[nodiscard]] auto GetParentGroup() const -> std::weak_ptr<Shape> { return parent_group_; }
  [[nodiscard]] virtual auto Includes(const Shape* object) const -> bool { return this == object; }
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
  [[nodiscard]] auto NormalAt(const Point& point, const Intersection& intersection) const -> Vec3 {
    const auto local_point = WorldToObject(point);
    const auto local_normal = LocalNormalAt(local_point, intersection);
    return NormalToWorld(local_normal).Normalize();
  }

  [[nodiscard]] auto ParentSpaceBounds() const -> BoundingBox { return BoundsOf().Transform(GetTransform()); }
  [[nodiscard]] virtual auto BoundsOf() const -> BoundingBox = 0;
  virtual void Divide(const int& threshold) { (void)threshold; }

 protected:
  [[nodiscard]] virtual auto LocalOccludes(const Ray& ray, const double max_distance) const -> bool {
    if (!GetCastShadow()) {
      return false;
    }
    const auto blocks_light = [max_distance](const Intersection& intersection) {
      const double hit_distance = intersection.GetTime();
      return hit_distance > utilities::kEpsilon && hit_distance < max_distance;
    };
    const auto intersections = LocalIntersect(ray);
    //does any intersection block any light? If so, return true. Otherwise, return false.
    return std::ranges::any_of(intersections, blocks_light);
  }
  virtual void InvalidateBounds() {  // NOLINT(misc-no-recursion)
    if (const auto parent = parent_group_.lock()) {
      parent->InvalidateBounds();
    }
  }

 private:
  std::size_t id_;
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
  Matrix<4, 4> inverse_transform_ = Matrix<4, 4>::IdentityMatrix();
  Matrix<4, 4> transpose_inverse_ = Matrix<4, 4>::IdentityMatrix();
  Material material_;
  bool cast_shadow_ = true;
  bool has_identity_transform_ = true;
  std::weak_ptr<Shape> parent_group_;
};

inline auto operator==(const Shape& lhs, const Shape& rhs) -> bool { return lhs.id_ == rhs.id_; }
}  // namespace ray_tracer
