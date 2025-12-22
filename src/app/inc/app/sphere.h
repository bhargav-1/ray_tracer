#pragma once
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "tuple.h"

namespace ray_tracer {

class Sphere {
 public:
  Sphere() = delete;
  constexpr explicit Sphere(std::size_t id) : id_(id) {} // NOLINT [readability-identifier-naming]
  ~Sphere() = default;
  [[nodiscard]] constexpr auto GetId() const noexcept -> std::size_t { return id_; }
  friend auto operator==(const Sphere& lhs, const Sphere& rhs) -> bool;
  [[nodiscard]] auto GetTransform() const -> Matrix<4, 4> { return transform_; }
  void SetTransform(const Matrix<4, 4>& transform) { transform_ = transform; }
  [[nodiscard]] auto NormalAt(const ray_tracer::Point& point) const -> Vec3 {
    const auto object_point = GetTransform().Inverse() * point;
    auto const object_normal = Vec3(object_point - ray_tracer::Point(0, 0, 0));
    // https:  //www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/transforming-normals.html
    Vec3 const world_normal = Transpose(GetTransform().Inverse()) * object_normal;
    return world_normal.Normalize();
  };
  [[nodiscard]] auto GetMaterial() const -> Material { return material_; }
  void SetMaterial(const Material& material) { material_ = material; }

 private:
  std::size_t id_;
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
  Material material_;
};

inline auto operator==(const Sphere& lhs, const Sphere& rhs) -> bool { return lhs.id_ == rhs.id_; }
}  // namespace ray_tracer
