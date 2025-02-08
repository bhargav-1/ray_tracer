#pragma once
#include "tuple.h"

namespace ray_tracer {
class Color : public Tuple {
 public:
  constexpr Color(double red, double green, double blue, double w = 0.0)
      : Tuple(red, green, blue, w), red_(red), green_(green), blue_(blue), w_(w) {}
  Color() : Color(1.0, 1.0, 1.0, 1.0) {}
  constexpr explicit Color(const Tuple& tuple) : Color(tuple.GetX(), tuple.GetY(), tuple.GetY(), tuple.GetW()) {}
  [[nodiscard]] constexpr auto GetRed() const -> double { return red_; }
  [[nodiscard]] constexpr auto GetGreen() const -> double { return green_; }
  [[nodiscard]] constexpr auto GetBlue() const -> double { return blue_; }
  [[nodiscard]] constexpr auto GetW() const -> double { return w_; }
  constexpr auto friend operator*(const Color& left, const Color& right) -> Color;
  constexpr auto operator*(const double& factor) const -> Color {
    return {red_ * factor, green_ * factor, blue_ * factor};
  }
  constexpr auto operator+(const Color& color) const -> Color {
    return {red_ + color.red_, green_ + color.green_, blue_ + color.blue_, w_ + color.w_};
  }
  // constexpr auto operator=(const Color& right) -> Color& {
  //   if (this != &right) {
  //     red_ = right.GetRed();
  //     green_ = right.GetGreen();
  //     blue_ = right.GetBlue();
  //     w_ = right.GetW();
  //   }
  //   return *this;
  // }

 private:
  double red_;
  double green_;
  double blue_;
  double w_;  // cppcheck-suppress [duplInheritedMember]
};
constexpr auto operator*(const Color& left, const Color& right) -> Color {
  return Color{left.red_ * right.red_, left.green_ * right.green_, left.blue_ * right.blue_, left.w_ * right.w_};
}
}  // namespace ray_tracer
