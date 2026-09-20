#pragma once
#include <cmath>
#include <limits>
#include <vector>
#include "material.h"
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "shape.h"
#include "tuple.h"

namespace ray_tracer {
class Intersection;
class Group : public Shape, public std::enable_shared_from_this<Group> {
 public:
  Group() = delete;
  constexpr Group(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Group() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  void AddChild(const std::shared_ptr<Shape>& child) {
    child->SetParentGroup(shared_from_this());
    children_.push_back(child);
  }
  auto GetChildren() const -> std::vector<std::shared_ptr<Shape>> { return children_; }
  auto BoundsOf() const -> BoundingBox override {
    BoundingBox box;
    for (const auto& child : children_) {
      box.AddPoint(child->ParentSpaceBounds().GetMin());
      box.AddPoint(child->ParentSpaceBounds().GetMax());
    }
    return box;
  }
 private:
  std::vector<std::shared_ptr<Shape>> children_;
};

inline auto operator==(const Group& lhs, const Group& rhs) -> bool { return lhs.GetId() == rhs.GetId(); }

inline auto Group::LocalIntersect(const Ray& transformed_ray) const -> std::vector<Intersection> {
  if (!BoundsOf().Intersects(transformed_ray)) {
    return {};
  }
  std::vector<Intersection> intersections;
  for (const auto& child : children_) {
    auto child_intersections = child->Intersect(transformed_ray);
    intersections.insert(intersections.end(), child_intersections.begin(), child_intersections.end());
  }
  std::ranges::sort(intersections, {}, &Intersection::GetTime);
  return intersections;
}

inline auto Group::LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 {
  (void)point;
  throw std::logic_error("a group has no surface normal");
}

}  // namespace ray_tracer
