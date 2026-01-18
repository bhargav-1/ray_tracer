#pragma once
#include "matrix.h"
#include "point.h"
#include "vec3.h"

namespace ray_tracer {
constexpr auto ViewTransform(const Point& from_point, const Point& to_point, const Vec3& up_vec) -> Matrix<4, 4> {
  const auto forward = Vec3(to_point - from_point).Normalize();
  const auto up_normal = up_vec.Normalize();
  const auto left = (forward.Cross(up_normal));
  const auto true_up = left.Cross(forward);
  const auto orientation = Matrix<4, 4>{{left.GetX(), left.GetY(), left.GetZ(), 0},
                                        {true_up.GetX(), true_up.GetY(), true_up.GetZ(), 0},
                                        {-forward.GetX(), -forward.GetY(), -forward.GetZ(), 0},
                                        {0, 0, 0, 1}};
  return orientation * Matrix<4, 4>::Translation(-from_point.GetX(), -from_point.GetY(), -from_point.GetZ());
}
}  // namespace ray_tracer