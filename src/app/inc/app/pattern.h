#pragma once
#include <utility>

#include "color.h"
#include "matrix.h"
#include "point.h"

namespace ray_tracer {
class Shape;
class Pattern {
 public:
  virtual ~Pattern() = default;
  [[nodiscard]] virtual auto ColorAt(const Point& point) const -> Color = 0;
  [[nodiscard]] auto GetPatternTransform() const -> Matrix<4, 4> { return transform_; }
  void SetPatternTransform(const Matrix<4, 4>& transform) { transform_ = transform; }

 private:
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
};

class StripePattern : public Pattern {
 public:
  StripePattern(Color color_a, Color color_b) : Pattern(), color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return color_a_; }
  [[nodiscard]] auto GetB() const -> Color { return color_b_; }
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color override {
    if (static_cast<int>(std::floor(point.GetX())) % 2 == 0) {
      return color_a_;
    }
    return color_b_;
  }

 private:
  Color color_a_;
  Color color_b_;
};

class GradientPattern : public Pattern {
public:
  GradientPattern(Color color_a, Color color_b) : Pattern(), color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return color_a_; }
  [[nodiscard]] auto GetB() const -> Color { return color_b_; }
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color override {
    const auto distance = color_b_ - color_a_;
    const auto fraction = (point.GetX() - std::floor(point.GetX()));
    return Color(color_a_ + distance * fraction);
  }

private:
  Color color_a_;
  Color color_b_;

};
class RingPattern: public Pattern {
public:
  RingPattern(Color color_a, Color color_b) : Pattern(), color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return color_a_; }
  [[nodiscard]] auto GetB() const -> Color { return color_b_; }
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color override {
    if (static_cast<int>(std::floor(std::sqrt((point.GetX() * point.GetX()) + (point.GetZ() * point.GetZ())))) % 2 == 0) {
      return color_a_;
    }
    return color_b_;
  }
private:
  Color color_a_;
  Color color_b_;
};

class CheckersPattern: public Pattern {
public:
  CheckersPattern(Color color_a, Color color_b) : Pattern(), color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return color_a_; }
  [[nodiscard]] auto GetB() const -> Color { return color_b_; }
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color override {
   if ((static_cast<int>(std::floor(point.GetX()) + std::floor(point.GetY()) + std::floor(point.GetZ()))) % 2 == 0) {
      return color_a_;
    }
    return color_b_;
  }
private:
  Color color_a_;
  Color color_b_;
};

}  // namespace ray_tracer
