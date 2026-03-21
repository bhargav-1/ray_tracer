#pragma once
#include <utility>

#include "color.h"
#include "matrix.h"
#include "point.h"

namespace ray_tracer{
class Shape;
class Pattern{
  public:
        virtual ~Pattern() = default;
        [[nodiscard]] virtual auto GetA() const -> Color = 0;
        [[nodiscard]] virtual auto GetB() const -> Color = 0;
        [[nodiscard]] virtual auto ColorAt(const Point& point) const -> Color = 0;
        [[nodiscard]] virtual auto GetPatternTransform() const -> Matrix<4, 4> =0;
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
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color override {
   if (static_cast<int>(std::floor(point.GetX())) % 2 == 0) {
     return color_a_;
   }
    return color_b_;
  }
  [[nodiscard]] auto GetPatternTransform() const -> Matrix<4, 4> override {
    return transform_;
  }
  void SetPatternTransform(const Matrix<4, 4>& transform) { transform_ = transform; }
private:
  Color color_a_;
  Color color_b_;
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
};

}