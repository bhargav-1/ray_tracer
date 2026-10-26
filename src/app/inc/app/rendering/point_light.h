#pragma once
#include <utility>

#include "app/materials/color.h"
#include "app/primitives/point.h"
#include "app/primitives/tuple.h"
#include "app/primitives/vec3.h"

namespace ray_tracer {
class PointLight {
 public:
  constexpr PointLight(Point position, Color intensity) noexcept
      : position_(std::move(position)), intensity_(std::move(intensity)) {}
  [[nodiscard]] constexpr auto GetPosition() const noexcept -> const Point& { return position_; }
  [[nodiscard]] constexpr auto GetIntensity() const noexcept -> const Color& { return intensity_; }
  friend auto operator==(const PointLight& lhs, const PointLight& rhs) -> bool {
    return lhs.position_ == rhs.position_ && lhs.intensity_ == rhs.intensity_;
  }

 private:
  Point position_;
  Color intensity_;
};
}  // namespace ray_tracer
