#include <iostream>
#include <numbers>
#include "app/canvas.h"
#include "app/matrix.h"
#include "app/point.h"
#include "app/tuple.h"

auto main() noexcept -> int {  // NOLINT
  ray_tracer::Canvas canvas{1200, 1200};
  constexpr ray_tracer::Point kCenter{600, 600, 0};
  canvas.WritePixelColor(
      static_cast<int>(kCenter.GetX()), static_cast<int>(kCenter.GetY()), ray_tracer::Color{255, 255, 255});
  constexpr ray_tracer::Point kOrigin{00, 00, 0};
  const ray_tracer::Matrix<4, 4> transform = ray_tracer::Matrix<4, 4>::IdentityMatrix().Translate(00, 300, 0);
  const ray_tracer::Point mark_12 = transform * kOrigin;
  ray_tracer::Point current_mark = mark_12;
  std::cout << "Mark12:" << '\n';
  std::cout << "x: " << current_mark.GetX() << ", y: " << current_mark.GetY() << ", z: " << current_mark.GetZ() << '\n';
  canvas.WritePixelColor(static_cast<int>(current_mark.GetX()),
                         canvas.GetHeight() - static_cast<int>(current_mark.GetY()),
                         ray_tracer::Color{255, 255, 255});
  auto temp_print_point = ray_tracer::Matrix<4, 4>::Translation(600, canvas.GetHeight() - 600, 0) * current_mark;
  canvas.WritePixelColor(static_cast<int>(temp_print_point.GetX()),
                         static_cast<int>(temp_print_point.GetY()),
                         ray_tracer::Color{255, 255, 255});
  while (ray_tracer::Matrix<4, 4>::RotationZ(std::numbers::pi / 6) * current_mark != mark_12) {
    current_mark = ray_tracer::Matrix<4, 4>::RotationZ(std::numbers::pi / 6) * current_mark;
    auto print_point = ray_tracer::Matrix<4, 4>::Translation(600, canvas.GetHeight() - 600, 0) * current_mark;
    canvas.WritePixelColor(
        static_cast<int>(print_point.GetX()), static_cast<int>(print_point.GetY()), ray_tracer::Color{255, 255, 255});
  }

  canvas.PrintPixels();
  canvas.CanvasToPPM("/tmp/clock_canvas.ppm");

  return 0;
}
