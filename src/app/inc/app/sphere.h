#pragma once
#include <vector>
#include "matrix.h"
#include "point.h"
#include "ray.h"
#include "tuple.h"

namespace ray_tracer {

template <std::size_t N>
class SphereCounter {
  friend class Sphere;
  static constexpr std::size_t kValue = N;
};

class Sphere {
 public:
  // __COUNTER__  is compiler specific and is used to generate a unique number for each sphere
  constexpr Sphere() noexcept : id_(SphereCounter<__COUNTER__>::kValue) {}
  ~Sphere() = default;
  [[nodiscard]] constexpr auto GetId() const noexcept -> std::size_t { return id_; }
  // can be constexpr in later cpp23++or cpp20++
  [[nodiscard]] inline static auto Intersect(const Ray& ray) -> std::vector<double>;

 private:
  std::size_t id_;
};

//en.wikipedia.org/wiki/Line–sphere_intersection
inline auto Sphere::Intersect(const Ray& ray) -> std::vector<double> {
  ray_tracer::Vec3 sphere_to_ray = ray.GetOrigin() - Point(0, 0, 0);
  auto a = ray.GetDirection().Dot(ray.GetDirection());
  auto b = 2 * ray.GetDirection().Dot(sphere_to_ray);
  auto c = sphere_to_ray.Dot(sphere_to_ray) - 1;
  auto discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return {};
  }
  // this t is time/unit in the ray
  auto t1 = (-b - std::sqrt(discriminant)) / (2 * a);
  auto t2 = (-b + std::sqrt(discriminant)) / (2 * a);
  return {t1, t2};
}
}  // namespace ray_tracer
