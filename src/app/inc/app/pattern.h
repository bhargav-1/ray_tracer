#pragma once
#include <memory>
#include <utility>

#include "color.h"
#include "matrix.h"
#include "point.h"
#include "utilities.h"

namespace ray_tracer {

class Shape;
class Pattern {
 public:
  virtual ~Pattern() = default;
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color {
    const auto local_point = inverse_transform_ * point;
    return LocalColorAt(local_point);
  }
  [[nodiscard]] virtual auto LocalColorAt(const Point& point) const -> Color = 0;
  [[nodiscard]] auto GetPatternTransform() const -> const Matrix<4, 4>& { return transform_; }
  void SetPatternTransform(const Matrix<4, 4>& transform) {
    transform_ = transform;
    inverse_transform_ = transform.Inverse();
  }

 private:
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
  Matrix<4, 4> inverse_transform_ = Matrix<4, 4>::IdentityMatrix();
};

class SolidPattern : public Pattern {
 public:
  explicit SolidPattern(Color color) : color_(std::move(color)) {}
  [[nodiscard]] auto LocalColorAt(const Point& /*point*/) const -> Color override { return color_; }

 private:
  Color color_;
};

inline auto MakeSolid(const Color& color) -> std::shared_ptr<Pattern> {
  return std::make_shared<SolidPattern>(color);
}

class StripePattern : public Pattern {
 public:
  StripePattern(Color color_a, Color color_b)
      : pattern_a_(MakeSolid(color_a)), pattern_b_(MakeSolid(color_b)) {}
  StripePattern(std::shared_ptr<Pattern> pattern_a, std::shared_ptr<Pattern> pattern_b)
      : pattern_a_(std::move(pattern_a)), pattern_b_(std::move(pattern_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return pattern_a_->ColorAt(Point(0, 0, 0)); }
  [[nodiscard]] auto GetB() const -> Color { return pattern_b_->ColorAt(Point(0, 0, 0)); }
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    if (static_cast<int>(std::floor(point.GetX() + utilities::kEpsilon)) % 2 == 0) {
      return pattern_a_->ColorAt(point);
    }
    return pattern_b_->ColorAt(point);
  }

 private:
  std::shared_ptr<Pattern> pattern_a_;
  std::shared_ptr<Pattern> pattern_b_;
};

class GradientPattern : public Pattern {
 public:
  GradientPattern(Color color_a, Color color_b) : color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return color_a_; }
  [[nodiscard]] auto GetB() const -> Color { return color_b_; }
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    const auto distance = color_b_ - color_a_;
    const auto fraction = (point.GetX() - std::floor(point.GetX() + utilities::kEpsilon));
    return Color(color_a_ + distance * fraction);
  }

 private:
  Color color_a_;
  Color color_b_;
};

class RingPattern : public Pattern {
 public:
  RingPattern(const Color& color_a, const Color& color_b)
      : pattern_a_(MakeSolid(color_a)), pattern_b_(MakeSolid(color_b)) {}
  RingPattern(std::shared_ptr<Pattern> pattern_a, std::shared_ptr<Pattern> pattern_b)
      : pattern_a_(std::move(pattern_a)), pattern_b_(std::move(pattern_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return pattern_a_->ColorAt(Point(0, 0, 0)); }
  [[nodiscard]] auto GetB() const -> Color { return pattern_b_->ColorAt(Point(0, 0, 0)); }
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    const auto radius = std::sqrt((point.GetX() * point.GetX()) + (point.GetZ() * point.GetZ()));
    if (static_cast<int>(std::floor(radius + utilities::kEpsilon)) % 2 == 0) {
      return pattern_a_->ColorAt(point);
    }
    return pattern_b_->ColorAt(point);
  }

 private:
  std::shared_ptr<Pattern> pattern_a_;
  std::shared_ptr<Pattern> pattern_b_;
};

class CheckersPattern : public Pattern {
 public:
  CheckersPattern(const Color& color_a, const Color& color_b)
      : pattern_a_(MakeSolid(color_a)), pattern_b_(MakeSolid(color_b)) {}
  CheckersPattern(std::shared_ptr<Pattern> pattern_a, std::shared_ptr<Pattern> pattern_b)
      : pattern_a_(std::move(pattern_a)), pattern_b_(std::move(pattern_b)) {}
  [[nodiscard]] auto GetA() const -> Color { return pattern_a_->ColorAt(Point(0, 0, 0)); }
  [[nodiscard]] auto GetB() const -> Color { return pattern_b_->ColorAt(Point(0, 0, 0)); }
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    if ((static_cast<int>(std::floor(point.GetX() + utilities::kEpsilon) +
                          std::floor(point.GetY() + utilities::kEpsilon) +
                          std::floor(point.GetZ() + utilities::kEpsilon))) % 2 == 0) {
      return pattern_a_->ColorAt(point);
    }
    return pattern_b_->ColorAt(point);
  }

 private:
  std::shared_ptr<Pattern> pattern_a_;
  std::shared_ptr<Pattern> pattern_b_;
};

class RadialGradientPattern : public Pattern {
 public:
  RadialGradientPattern(Color color_a, Color color_b) : color_a_(std::move(color_a)), color_b_(std::move(color_b)) {}
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    const auto distance = color_b_ - color_a_;
    const auto radius = std::sqrt((point.GetX() * point.GetX()) + (point.GetZ() * point.GetZ()));
    const auto fraction = radius - std::floor(radius + utilities::kEpsilon);
    return Color(color_a_ + distance * fraction);
  }

 private:
  Color color_a_;
  Color color_b_;
};

class BlendedPattern : public Pattern {
 public:
  BlendedPattern(std::shared_ptr<Pattern> pattern_a, std::shared_ptr<Pattern> pattern_b)
      : pattern_a_(std::move(pattern_a)), pattern_b_(std::move(pattern_b)) {}
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    const auto col_a = pattern_a_->ColorAt(point);
    const auto col_b = pattern_b_->ColorAt(point);
    return Color((col_a + col_b) * 0.5);
  }

 private:
  std::shared_ptr<Pattern> pattern_a_;
  std::shared_ptr<Pattern> pattern_b_;
};

}  // namespace ray_tracer
