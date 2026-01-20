#pragma once
#include <numbers>
#include "matrix.h"
#include "point.h"
#include "vec3.h"
#include "ray.h"
namespace ray_tracer {

class Camera {
 public:
  constexpr Camera(const int horizontal_size, const int vertical_size, const double field_of_view)
      : horizontal_size_(horizontal_size), vertical_size_(vertical_size), field_of_view_(field_of_view) {
    auto const half_view = std::tan(field_of_view_ / 2);
    aspect_ratio_ = static_cast<double>(horizontal_size_) / vertical_size_;
    if (aspect_ratio_ >= 1) {
      half_width_ = half_view;
      half_height_ = half_view / aspect_ratio_;
    } else {
      half_width_ = half_view * aspect_ratio_;
      half_height_ = half_view;
    }
    pixel_size_ = (half_width_ * 2) / horizontal_size_;
  };

  [[nodiscard]] constexpr auto GetHorizontalSize() const -> int { return horizontal_size_; }
  [[nodiscard]] constexpr auto GetVerticalSize() const -> int { return vertical_size_; }
  [[nodiscard]] constexpr auto GetFieldOfView() const -> double { return field_of_view_; }
  [[nodiscard]] constexpr auto GetTransform() const -> ray_tracer::Matrix<4, 4> {
    return transform_;
  }
  constexpr void SetTransform(const ray_tracer::Matrix<4, 4>& transform) { transform_ = transform; }
  [[nodiscard]] constexpr auto GetRayForPixel(const int pixel_x, const int pixel_y) -> ray_tracer::Ray {
    auto const x_offset = (pixel_x + 0.5) * pixel_size_;
    auto const y_offset = (pixel_y + 0.5) * pixel_size_;
    auto const world_x = half_width_ - x_offset;
    auto const world_y = half_height_ - y_offset;
    const Point pixel = transform_.Inverse() * ray_tracer::Point(world_x, world_y, -1);
    Point const origin= transform_.Inverse() * ray_tracer::Point(0,0, 0);
    return {origin, Vec3(pixel- origin).Normalize()};
  }

  [[nodiscard]] constexpr auto GetPixelSize() const -> double { return pixel_size_; }

 private:
  int horizontal_size_;
  int vertical_size_;
  double field_of_view_;
  double half_width_;
  double half_height_;
  double aspect_ratio_;
  double pixel_size_;
  ray_tracer::Matrix<4, 4> transform_ = ray_tracer::Matrix<4, 4>::IdentityMatrix();
};
}  // namespace ray_tracer