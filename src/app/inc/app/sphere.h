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
  friend auto operator==(const Sphere& lhs, const Sphere& rhs) -> bool;
  [[nodiscard]] auto GetTransform() const -> Matrix<4, 4> { return transform_; }
  void SetTransform(const Matrix<4, 4>& transform) { transform_ = transform; }

 private:
  std::size_t id_;
  Matrix<4, 4> transform_ = Matrix<4, 4>::IdentityMatrix();
};

inline auto operator==(const Sphere& lhs, const Sphere& rhs) -> bool { return lhs.id_ == rhs.id_; }
}  // namespace ray_tracer
