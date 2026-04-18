#pragma once

#include <utility>

#include "point.h"
#include "shape.h"
#include "vec3.h"

namespace ray_tracer {

class Computations {
 public:
  constexpr Computations(const double time, const Shape* object, Point point, Vec3 eye_v, Vec3 normal_v, bool inside, Point over_point, Vec3 reflect_v)
      : time_(time),
        object_(object),
        point_(std::move(point)),
        eye_v_(std::move(eye_v)),
        normal_v_(std::move(normal_v)),
        inside_(inside),
        over_point_(std::move(over_point)),
        reflect_v_(std::move(reflect_v)){}

  [[nodiscard]] constexpr auto GetTime() const -> double { return time_; }
  [[nodiscard]] constexpr auto GetObject() const -> const Shape* { return object_; }
  [[nodiscard]] constexpr auto GetPoint() const -> Point { return point_; }
  [[nodiscard]] constexpr auto GetEyeV() const -> Vec3 { return eye_v_; }
  [[nodiscard]] constexpr auto GetNormalV() const -> Vec3 { return normal_v_; }
  [[nodiscard]] constexpr auto GetOverPoint() const -> Point { return over_point_; }
  [[nodiscard]] constexpr auto IsInside() const -> bool { return inside_; }
  [[nodiscard]] constexpr auto GetReflectV() const -> Vec3 { return reflect_v_; }

 private:
  double time_;
  const Shape* object_;
  Point point_;
  Vec3 eye_v_;
  Vec3 normal_v_;
  bool inside_;
  Point over_point_;
  Vec3 reflect_v_;
};

}  // namespace ray_tracer
