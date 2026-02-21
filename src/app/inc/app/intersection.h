#pragma once

namespace ray_tracer {
class Shape;

class Intersection {
 public:
  Intersection(double time, const Shape& shape) : time_(time), shape_(&shape) {}
  auto friend operator==(Intersection const& lhs, Intersection const& rhs) -> bool;
  [[nodiscard]] constexpr auto GetTime() const { return time_; }
  [[nodiscard]] auto GetObject() const { return shape_; }

 private:
  double time_;
  const Shape* shape_;
};

inline auto operator==(Intersection const& lhs, Intersection const& rhs) -> bool {
  return lhs.time_ == rhs.time_ && lhs.shape_ == rhs.shape_;
}

}  // namespace ray_tracer
