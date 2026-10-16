#pragma once
#include <cmath>
#include <limits>
#include <optional>
#include <utility>
#include <vector>
#include "app/materials/material.h"
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/rendering/ray.h"
#include "app/shapes/shape.h"
#include "app/primitives/tuple.h"
#include "app/primitives/utilities.h"

namespace ray_tracer {
class Intersection;
class Group : public Shape, public std::enable_shared_from_this<Group> {
 public:
  Group() = delete;
  constexpr Group(size_t id) : Shape(id) {}  // NOLINT [readability-identifier-naming]
  ~Group() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override;
  [[nodiscard]] auto LocalOccludes(const Ray& ray, double max_distance) const -> bool override;
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override;
  void AddChild(const std::shared_ptr<Shape>& child) {
    child->SetParentGroup(shared_from_this());
    children_.push_back(child);
    InvalidateBounds();
  }
  auto GetChildren() const -> std::vector<std::shared_ptr<Shape>> { return children_; }
  [[nodiscard]] auto Includes(const Shape* object) const -> bool override {  // NOLINT(misc-no-recursion)
    return Shape::Includes(object) || std::ranges::any_of(children_, [object](const auto& child) {
      return child->Includes(object);
    });
  }
  auto BoundsOf() const -> BoundingBox override {
    if (cached_bounds_.has_value()) {
      return *cached_bounds_;
    }
    BoundingBox box;
    for (const auto& child : children_) {
      const auto child_bounds = child->ParentSpaceBounds();
      box.AddPoint(child_bounds.GetMin());
      box.AddPoint(child_bounds.GetMax());
    }
    cached_bounds_ = box;
    return box;
  }
  auto PartitionChildren() -> std::pair<std::vector<std::shared_ptr<Shape>>, std::vector<std::shared_ptr<Shape>>> {
    auto box = BoundsOf();
    auto [left_box, right_box] = box.SplitBounds();
    std::vector<std::shared_ptr<Shape>> left_children;
    std::vector<std::shared_ptr<Shape>> right_children;
    std::vector<std::shared_ptr<Shape>> remaining_children;
    for (const auto& child : children_) {
      if (left_box.Contains(child->ParentSpaceBounds())) {
        left_children.push_back(child);
      } else if (right_box.Contains(child->ParentSpaceBounds())) {
        right_children.push_back(child);
      } else {
        remaining_children.push_back(child);
      }
    }
    children_ = std::move(remaining_children);
    InvalidateBounds();
    return {left_children, right_children};
  }
  auto MakeSubGroup(const std::vector<std::shared_ptr<Shape>>& children) -> std::shared_ptr<Group> {
    auto sub_group = std::make_shared<Group>(GetId());
    for (const auto& child : children) {
      sub_group->AddChild(child);
    }
    AddChild(sub_group);
    return sub_group;
  }
  void Divide(const int &threshold) override {
    if (std::cmp_less_equal(threshold ,GetChildren().size())) {
        auto [left_children, right_children] = PartitionChildren();
        if (!left_children.empty()) {
          MakeSubGroup(left_children);
        }
        if (!right_children.empty()) {
          MakeSubGroup(right_children);
        }
    }
    for (const auto& child : children_) {
      child->Divide(threshold);
    }
  }
 private:
  void InvalidateBounds() override {
    cached_bounds_.reset();
    Shape::InvalidateBounds();
  }
  mutable std::optional<BoundingBox> cached_bounds_;
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

inline auto Group::LocalOccludes(const Ray& ray, const double max_distance) const -> bool {
  double entry = 0;
  double exit = 0;
  if (!BoundsOf().Intersects(ray, entry, exit) || exit <= utilities::kEpsilon || entry >= max_distance) {
    return false;
  }
  return std::ranges::any_of(children_, [&](const std::shared_ptr<Shape>& child) {
    return child->Occludes(ray, max_distance);
  });
}

}  // namespace ray_tracer
