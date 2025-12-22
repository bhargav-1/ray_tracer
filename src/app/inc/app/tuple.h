#pragma once
#include <app/utilities.h>
#include <iostream>  // for operator<<, basic_ostream, basic_ostream<>::__os...

namespace ray_tracer {
class Tuple {
 public:
  constexpr Tuple() noexcept = default;
  constexpr Tuple(const double& x_value, const double& y_value, const double& z_value, const double& w_value);
  constexpr Tuple(const Tuple&) = default;
  constexpr auto operator=(const Tuple& tuple) -> Tuple& {
    if (this != &tuple) {
      x_ = tuple.GetX();
      y_ = tuple.GetY();
      z_ = tuple.GetZ();
      w_ = tuple.GetW();
    }
    return *this;
  }

  [[nodiscard]] constexpr auto IsPoint() const -> bool;
  [[nodiscard]] constexpr auto IsVector() const -> bool;
  // can use this for cpp20 -once ubuntu is updated
  //@todo: friend auto operator==(const Tuple&, const Tuple&) -> bool = default;
  constexpr auto friend operator==(const Tuple& left, const Tuple& right) -> bool;
  constexpr auto friend operator!=(const Tuple& left, const Tuple& right) -> bool;

  constexpr auto friend operator+(const Tuple& left, const Tuple& right) -> Tuple;
  constexpr auto friend operator-(const Tuple& left, const Tuple& right) -> Tuple;
  constexpr auto friend operator-(const Tuple& tuple) -> Tuple;
  constexpr auto friend operator*(const Tuple& tuple, const double& scale) -> Tuple;
  constexpr auto friend operator*(const double& scale, const Tuple& tuple) -> Tuple;
  constexpr auto friend operator/(const Tuple& tuple, const double& scale) -> Tuple;
  auto friend operator<<(std::ostream& output_stream, const Tuple& tuple) -> std::ostream&;
  [[nodiscard]] constexpr auto GetX() const -> double { return x_; }
  [[nodiscard]] constexpr auto GetY() const -> double { return y_; }
  [[nodiscard]] constexpr auto GetZ() const -> double { return z_; }
  [[nodiscard]] constexpr auto GetW() const -> double { return w_; }

 protected:
  double x_{0};  // NOLINT
  double y_{0};  // NOLINT
  double z_{0};  // NOLINT
  double w_{0};  // NOLINT
};
constexpr Tuple::Tuple(const double& x_value, const double& y_value, const double& z_value, const double& w_value)
    : x_(x_value), y_(y_value), z_(z_value), w_(w_value) {}
constexpr auto operator==(const Tuple& left, const Tuple& right) -> bool {
  return (utilities::Equal(left.x_, right.x_) && utilities::Equal(left.y_, right.y_) &&
          utilities::Equal(left.z_, right.z_) && utilities::Equal(left.w_, right.w_));
}
constexpr auto operator!=(const Tuple& left, const Tuple& right) -> bool {
  return (!utilities::Equal(left.x_, right.x_) || !utilities::Equal(left.y_, right.y_) ||
          !utilities::Equal(left.z_, right.z_) || !utilities::Equal(left.w_, right.w_));
}

constexpr auto operator+(const Tuple& left, const Tuple& right) -> Tuple {
  return Tuple{left.x_ + right.x_, left.y_ + right.y_, left.z_ + right.z_, left.w_ + right.w_};
}
constexpr auto operator-(const Tuple& left, const Tuple& right) -> Tuple {
  return Tuple{left.x_ - right.x_, left.y_ - right.y_, left.z_ - right.z_, left.w_ - right.w_};
}
constexpr auto operator-(const Tuple& tuple) -> Tuple { return Tuple{-tuple.x_, -tuple.y_, -tuple.z_, -tuple.w_}; }
constexpr auto operator*(const Tuple& tuple, const double& scale) -> Tuple {
  return Tuple{scale * tuple.x_, scale * tuple.y_, scale * tuple.z_, scale * tuple.w_};
}
constexpr auto operator*(const double& scale, const Tuple& tuple) -> Tuple { return operator*(tuple, scale); }

constexpr auto operator/(const Tuple& tuple, const double& scale) -> Tuple {
  return Tuple{tuple.x_ / scale, tuple.y_ / scale, tuple.z_ / scale, tuple.w_ / scale};
}
inline auto operator<<(std::ostream& output_stream, const Tuple& tuple) -> std::ostream& {
  output_stream << "x: " << tuple.x_ << ", y: " << tuple.y_ << ", z: " << tuple.z_ << ", w: " << tuple.w_;
  return output_stream;
}

constexpr auto Tuple::IsPoint() const -> bool { return w_ == 1; }
constexpr auto Tuple::IsVector() const -> bool { return w_ == 0; }

}  // namespace ray_tracer
