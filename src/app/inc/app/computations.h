#pragma once

#include <utility>

#include "point.h"
#include "shape.h"
#include "vec3.h"

namespace ray_tracer {

class Computations {
 public:
  constexpr Computations(const double time,
                         const Shape* object,
                         Point point,
                         Vec3 eye_v,
                         Vec3 normal_v,
                         bool inside,
                         Point over_point,
                         Vec3 reflect_v,
                         double refractive_index_n1,
                         double refractive_index_n2,
                         Point under_point)
      : time_(time),
        object_(object),
        point_(std::move(point)),
        eye_v_(std::move(eye_v)),
        normal_v_(std::move(normal_v)),
        inside_(inside),
        over_point_(std::move(over_point)),
        reflect_v_(std::move(reflect_v)),
        refractive_index_n1_(refractive_index_n1),
        refractive_index_n2_(refractive_index_n2),
        under_point_(under_point) {}

  [[nodiscard]] constexpr auto GetTime() const -> double { return time_; }
  [[nodiscard]] constexpr auto GetObject() const -> const Shape* { return object_; }
  [[nodiscard]] constexpr auto GetPoint() const -> Point { return point_; }
  [[nodiscard]] constexpr auto GetEyeV() const -> Vec3 { return eye_v_; }
  [[nodiscard]] constexpr auto GetNormalV() const -> Vec3 { return normal_v_; }
  [[nodiscard]] constexpr auto GetOverPoint() const -> Point { return over_point_; }
  [[nodiscard]] constexpr auto IsInside() const -> bool { return inside_; }
  [[nodiscard]] constexpr auto GetReflectV() const -> Vec3 { return reflect_v_; }
  [[nodiscard]] constexpr auto GetRefractiveIndexN1() const -> double { return refractive_index_n1_; }
  [[nodiscard]] constexpr auto GetRefractiveIndexN2() const -> double { return refractive_index_n2_; }
  [[nodiscard]] constexpr auto GetUnderPoint() const -> Point { return under_point_; }

 private:
  double time_;
  const Shape* object_;
  Point point_;
  Vec3 eye_v_;
  Vec3 normal_v_;
  bool inside_;
  Point over_point_;
  Vec3 reflect_v_;
  double refractive_index_n1_;
  double refractive_index_n2_;
  Point under_point_;
};

}  // namespace ray_tracer
