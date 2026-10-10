#pragma once

namespace ray_tracer {
class Shape;

class Intersection {
 public:
  Intersection(double time, const Shape& shape, double u_value = 0, double v_value = 0)
      : time_(time), shape_(&shape), u_value_(u_value), v_value_(v_value) {}
  auto friend operator==(Intersection const& lhs, Intersection const& rhs) -> bool;
  [[nodiscard]] constexpr auto GetTime() const { return time_; }
  [[nodiscard]] auto GetObject() const { return shape_; }
  [[nodiscard]] constexpr auto GetU() const -> double { return u_value_; }
  [[nodiscard]] constexpr auto GetV() const -> double { return v_value_; }

 private:
  double time_;
  const Shape* shape_;
  double u_value_;
  double v_value_;
};

inline auto operator==(Intersection const& lhs, Intersection const& rhs) -> bool {
  return lhs.time_ == rhs.time_ && lhs.shape_ == rhs.shape_;
}

}  // namespace ray_tracer
