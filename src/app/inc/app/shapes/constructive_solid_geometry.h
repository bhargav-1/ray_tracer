#pragma once

#include <memory>
#include "app/shapes/shape.h"
namespace ray_tracer {

enum class CSGOperation : std::uint8_t { kUnion, kIntersection, kDifference };

class ConstructiveSolidGeometry : public Shape {
 public:
  ConstructiveSolidGeometry() = delete;
  ~ConstructiveSolidGeometry() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override {
    if (!BoundsOf().Intersects(ray)) {
      return {};
    }
    auto left_intersections = left_->Intersect(ray);
    auto right_intersections = right_->Intersect(ray);
    std::vector<Intersection> all_intersections = left_intersections;
    all_intersections.insert(all_intersections.end(), right_intersections.begin(), right_intersections.end());
    std::ranges::sort(all_intersections, {}, &Intersection::GetTime);
    return FilterIntersections(all_intersections);
  }
  [[nodiscard]] auto LocalNormalAt(const Point& point) const -> Vec3 override {
    (void)point;
    throw std::logic_error("CSG has no surface normal");
  }
  [[nodiscard]] auto GetLeft() const -> const std::shared_ptr<Shape>& { return left_; }
  [[nodiscard]] auto GetRight() const -> const std::shared_ptr<Shape>& { return right_; }
  [[nodiscard]] auto GetOperation() const -> CSGOperation { return operation_; }
  [[nodiscard]] auto Includes(const Shape* object) const -> bool override {  // NOLINT(misc-no-recursion)
    return Shape::Includes(object) || left_->Includes(object) || right_->Includes(object);
  }
  static auto IntersectionAllowed(const CSGOperation operation, const bool lhit, const bool inl, const bool inr)
      -> bool {
    switch (operation) {
      case CSGOperation::kUnion:
        return (lhit && !inr) || (!lhit && !inl);
      case CSGOperation::kIntersection:
        return (lhit && inr) || (!lhit && inl);
      case CSGOperation::kDifference:
        return (lhit && !inr) || (!lhit && inl);
      default:
        throw std::logic_error("Unknown CSG operation");
    }
  }
  auto FilterIntersections(const std::vector<Intersection>& intersections) const -> std::vector<Intersection> {
    bool inl = false;
    bool inr = false;
    std::vector<Intersection> result;
    for (const auto& intersection : intersections) {
      const bool lhit = left_->Includes(intersection.GetObject());
      if (IntersectionAllowed(operation_, lhit, inl, inr)) {
        result.push_back(intersection);
      }
      if (lhit) {
        inl = !inl;
      } else {
        inr = !inr;
      }
    }
    return result;
  }
  [[nodiscard]] auto BoundsOf() const -> BoundingBox override {
    if (cached_bounds_.has_value()) {
      return *cached_bounds_;
    }
    BoundingBox box;
    for (const auto& child : {left_, right_}) {
      const auto child_bounds = child->ParentSpaceBounds();
      box.AddPoint(child_bounds.GetMin());
      box.AddPoint(child_bounds.GetMax());
    }
    cached_bounds_ = box;
    return box;
  }
  static auto CreateCSG(size_t shape_id,
                        std::shared_ptr<Shape> left,
                        std::shared_ptr<Shape> right,
                        CSGOperation operation) -> std::shared_ptr<ConstructiveSolidGeometry> {
    auto csg = std::shared_ptr<ConstructiveSolidGeometry>(
        new ConstructiveSolidGeometry(shape_id, std::move(left), std::move(right), operation));
    csg->left_->SetParentGroup(csg);
    csg->right_->SetParentGroup(csg);
    return csg;
  }
  void Divide(const int& threshold) override {  // NOLINT(misc-no-recursion)
    left_->Divide(threshold);
    right_->Divide(threshold);
  }

 private:
  explicit ConstructiveSolidGeometry(const std::size_t shape_id,
                                     std::shared_ptr<Shape> left,
                                     std::shared_ptr<Shape> right,
                                     CSGOperation operation)
      : Shape(shape_id), left_(std::move(left)), right_(std::move(right)), operation_(operation) {}

  void InvalidateBounds() override {
    cached_bounds_.reset();
    Shape::InvalidateBounds();
  }
  std::shared_ptr<Shape> left_;
  std::shared_ptr<Shape> right_;
  std::shared_ptr<Shape> parent_;
  CSGOperation operation_;
  mutable std::optional<BoundingBox> cached_bounds_;
};

}  // namespace ray_tracer
