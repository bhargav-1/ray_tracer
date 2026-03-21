#pragma once
#include <utility>

#include "color.h"
#include "point.h"

namespace ray_tracer{
class Pattern{
  public:
        virtual ~Pattern() = default;
        [[nodiscard]] virtual auto GetA() const -> Color = 0;
        [[nodiscard]] virtual auto GetB() const -> Color = 0;
};

class StripePattern : public Pattern {
public:
  StripePattern(Color  color_a, Color  color_b): Pattern(), color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto GetA() const -> Color override {
    return color_a_;
  }
  [[nodiscard]] auto GetB() const -> Color override {
    return color_b_;
  }
  [[nodiscard]] auto StripeAt(const Point& point) const -> Color {
   if (static_cast<int>(std::floor(point.GetX())) % 2 == 0) {
     return color_a_;
   }
    return color_b_;
  }
private:
  Color color_a_;
  Color color_b_;
};
}