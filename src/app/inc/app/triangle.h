#pragma once
#include <algorithm>
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
class Triangle : public Shape {
 public:
  Triangle() = delete;
  constexpr Triangle(size_t id, std::array<Point, 3> vertices) : Shape(id), vertices_(vertices) {  // NOLINT
    edge1_ = vertices_[1] - vertices_[0];
    edge2_ = vertices_[2] - vertices_[0];
    normal_ = edge2_.Cross(edge1_).Normalize();
  }
  ~Triangle() override = default;
  [[nodiscard]] auto GetVertices() const -> std::array<Point, 3> { return vertices_; }
  [[nodiscard]] auto GetNormal() const -> Vec3 { return normal_; }
  [[nodiscard]] auto GetEdge1() const -> Vec3 { return edge1_; }
  [[nodiscard]] auto GetEdge2() const -> Vec3 { return edge2_; }
  [[nodiscard]] auto LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  [[nodiscard]] auto BoundsOf() const -> BoundingBox override;

 private:
  std::array<Point, 3> vertices_;
  Vec3 edge1_;
  Vec3 edge2_;
  Vec3 normal_;
};

inline auto operator==(const Triangle& lhs, const Triangle& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }
inline auto Triangle::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  if (!BoundsOf().Intersects(transformed_ray)) {
    return {};
  }
  auto determinant = transformed_ray.GetDirection().Cross(edge2_).Dot(edge1_);
  if (std::abs(determinant) < utilities::kEpsilon) {
    return {};
  }
  auto f_value = 1.0/determinant;
  auto p1_to_origin = Vec3(transformed_ray.GetOrigin() - vertices_[0]);
  auto u_value = f_value * transformed_ray.GetDirection().Cross(edge2_).Dot(p1_to_origin);
  if (u_value < 0.0 || u_value > 1.0) {
    return {};
  }
  auto origin_cross_e1 = p1_to_origin.Cross(edge1_);
  auto v_value = f_value * transformed_ray.GetDirection().Dot(origin_cross_e1);
  if (v_value < 0.0 || (u_value + v_value) > 1.0) {
    return {};
  }
  auto time = f_value * edge2_.Dot(origin_cross_e1);
  return {Intersection(time, *this)};
}

inline auto Triangle::LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 {
  (void)point;  // Unused parameter
  return normal_;
}
inline auto Triangle::BoundsOf() const -> BoundingBox {
  BoundingBox bounds;
  for (const auto& vertex : vertices_) {
    bounds.AddPoint(vertex);
  }
  return bounds;
}
}  // namespace ray_tracer
