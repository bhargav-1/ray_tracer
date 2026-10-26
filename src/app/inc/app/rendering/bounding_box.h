#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/utilities.h"
#include "app/rendering/ray.h"

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

  [[nodiscard]] auto Intersects(const Ray& ray, double& tmin, double& tmax) const -> bool {
    const auto is_parallel_outside =
        [](const double origin, const double direction, const double min, const double max) {
          return std::abs(direction) < utilities::kEpsilon && (origin < min || origin > max);
        };
    if (is_parallel_outside(ray.GetOrigin().GetX(), ray.GetDirection().GetX(), min_.GetX(), max_.GetX()) ||
        is_parallel_outside(ray.GetOrigin().GetY(), ray.GetDirection().GetY(), min_.GetY(), max_.GetY()) ||
        is_parallel_outside(ray.GetOrigin().GetZ(), ray.GetDirection().GetZ(), min_.GetZ(), max_.GetZ())) {
      return false;
    }

    const auto x_axis = CheckAxis(ray.GetOrigin().GetX(), ray.GetDirection().GetX(), min_.GetX(), max_.GetX());
    const auto y_axis = CheckAxis(ray.GetOrigin().GetY(), ray.GetDirection().GetY(), min_.GetY(), max_.GetY());
    tmin = std::max(x_axis.first, y_axis.first);
    tmax = std::min(x_axis.second, y_axis.second);
    if (tmin > tmax) {
      return false;  // No X/Y overlap: the Z interval cannot make this a hit.
    }
    const auto z_axis = CheckAxis(ray.GetOrigin().GetZ(), ray.GetDirection().GetZ(), min_.GetZ(), max_.GetZ());
    tmin = std::max(tmin, z_axis.first);
    tmax = std::min(tmax, z_axis.second);
    return tmin <= tmax;
  }

  [[nodiscard]] auto Intersects(const Ray& ray) const -> bool {
    double tmin = 0;
    double tmax = 0;
    return Intersects(ray, tmin, tmax);
  }
  [[nodiscard]] auto SplitBounds() const -> std::pair<BoundingBox, BoundingBox> {
    const auto x_span = max_.GetX() - min_.GetX();
    const auto y_span = max_.GetY() - min_.GetY();
    const auto z_span = max_.GetZ() - min_.GetZ();
    const auto max_span = std::max({x_span, y_span, z_span});
    auto x0_value = min_.GetX();
    auto y0_value = min_.GetY();
    auto z0_value = min_.GetZ();
    auto x1_value = max_.GetX();
    auto y1_value = max_.GetY();
    auto z1_value = max_.GetZ();

    if (utilities::Equal(max_span, x_span)) {
      const auto x_mid = (min_.GetX() + max_.GetX()) / 2;
      x0_value = x_mid;
      x1_value = x_mid;
    } else if (utilities::Equal(max_span, y_span)) {
      const auto y_mid = (min_.GetY() + max_.GetY()) / 2;
      y0_value = y_mid;
      y1_value = y_mid;
    } else {
      const auto z_mid = (min_.GetZ() + max_.GetZ()) / 2;
      z0_value = z_mid;
      z1_value = z_mid;
    }
    auto mid_min = Point(x0_value, y0_value, z0_value);
    auto mid_max = Point(x1_value, y1_value, z1_value);
    return {BoundingBox(min_, mid_max), BoundingBox(mid_min, max_)};
  }

  [[nodiscard]] auto operator==(const BoundingBox& other) const -> bool {
    return min_ == other.min_ && max_ == other.max_;
  }

 private:
  [[nodiscard]] static auto CheckAxis(double origin,
                                      double direction,
                                      double min,
                                      double max) -> std::pair<double, double> {
    auto tmin_numerator = min - origin;
    auto tmax_numerator = max - origin;
    double tmin = 0;
    double tmax = 0;
    if (std::abs(direction) >= utilities::kEpsilon) {
      tmin = tmin_numerator / direction;
      tmax = tmax_numerator / direction;
    } else {
      tmin = tmin_numerator * std::numeric_limits<double>::infinity();
      tmax = tmax_numerator * std::numeric_limits<double>::infinity();
    }
    if (tmin > tmax) {
      std::swap(tmin, tmax);
    }
    return {tmin, tmax};
  }

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
