#pragma once
#include <algorithm>
#include <limits>
#include <vector>

#include "app/point.h"
#include "app/matrix.h"

namespace ray_tracer {
class BoundingBox {
 public:
  BoundingBox() = default;
  BoundingBox(const Point& min_point, const Point& max_point) {
    max_ = max_point;
    min_ = min_point;
  }
  void AddPoint(const Point& point) {
    max_ = Point(
        std::max(max_.GetX(), point.GetX()), std::max(max_.GetY(), point.GetY()), std::max(max_.GetZ(), point.GetZ()));
    min_ = Point(
        std::min(min_.GetX(), point.GetX()), std::min(min_.GetY(), point.GetY()), std::min(min_.GetZ(), point.GetZ()));
  }
  void AddPoint(const std::vector<Point>& points) {
    for (const auto& point : points) {
      AddPoint(point);
    }
  }
  [[nodiscard]] auto GetMax() const -> Point { return max_; }
  [[nodiscard]] auto GetMin() const -> Point { return min_; }
  [[nodiscard]] auto Contains(const Point& point) const -> bool {
    return point.GetX() >= min_.GetX() && point.GetX() <= max_.GetX() && point.GetY() >= min_.GetY() &&
           point.GetY() <= max_.GetY() && point.GetZ() >= min_.GetZ() && point.GetZ() <= max_.GetZ();
  }
  [[nodiscard]] auto Contains(const BoundingBox& box) const -> bool {
    return Contains(box.GetMin()) && Contains(box.GetMax());
  }
  [[nodiscard]] auto Transform(Matrix<4, 4> transform) const -> BoundingBox {
    auto points = std::vector<Point>{
        Point(min_.GetX(), min_.GetY(), min_.GetZ()),
        Point(min_.GetX(), min_.GetY(), max_.GetZ()),
        Point(min_.GetX(), max_.GetY(), min_.GetZ()),
        Point(min_.GetX(), max_.GetY(), max_.GetZ()),
        Point(max_.GetX(), min_.GetY(), min_.GetZ()),
        Point(max_.GetX(), min_.GetY(), max_.GetZ()),
        Point(max_.GetX(), max_.GetY(), min_.GetZ()),
        Point(max_.GetX(), max_.GetY(), max_.GetZ()),
    };
    BoundingBox transformed_box;
    for (const auto& point : points) {
      transformed_box.AddPoint(transform * point);
    }
    return transformed_box;
  }

 private:
  Point max_{-1 * std::numeric_limits<double>::infinity(),
             -1 * std::numeric_limits<double>::infinity(),
             -1 * std::numeric_limits<double>::infinity()};
  Point min_{std::numeric_limits<double>::infinity(),
             std::numeric_limits<double>::infinity(),
             std::numeric_limits<double>::infinity()};
};
inline auto operator+(BoundingBox& lhs, BoundingBox& rhs) -> BoundingBox {
  lhs.AddPoint(rhs.GetMin());
  lhs.AddPoint(rhs.GetMax());
  return lhs;
}
}  // namespace ray_tracer