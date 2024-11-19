#include <iostream>
#include "app/canvas.h"
#include "app/point.h"
#include "app/tuple.h"
#include "app/vec3.h"

struct Environment {
  ray_tracer::Vec3 gravity;
  ray_tracer::Point wind;
};
struct Projectile {
  ray_tracer::Point position;
  ray_tracer::Vec3 velocity;
};

void Tick(const Environment& env, Projectile& proj) { // NOLINT
  proj.velocity = proj.velocity + env.gravity + env.wind;
  proj.position = proj.position + proj.velocity;
  std::cout << "x: " << proj.position.GetX() << ", y: " << proj.position.GetY() << ", z: " << proj.position.GetZ()
            << '\n';
  std::cout << "-  - - -- - - - -- -- - -- - - - - - -- -" << '\n';
}

auto main() noexcept -> int {  // NOLINT
  Projectile projtl{.position = ray_tracer::Point{0, 1, 0},
                    .velocity = ray_tracer::Vec3{1, 1.8, 0}.Normalize() * 11.25};
  Environment const env{.gravity = ray_tracer::Vec3{0, -0.1, 0}, .wind = ray_tracer::Vec3{-0.01, 0, 0}};
  int count = 0;
  std::cout << "!!!x: " << projtl.position.GetX() << ", y: " << projtl.position.GetY()
            << ", z: " << projtl.position.GetZ() << '\n';
  ray_tracer::Canvas canvas{900, 550};
  while (projtl.position.GetY() >= -1) {
    canvas.WritePixelColor(static_cast<int>(projtl.position.GetX()),
                           canvas.GetHeight() - static_cast<int>(projtl.position.GetY()),
                           ray_tracer::Color{1, 0, 0});
    Tick(env, projtl);
    count++;
  }
  canvas.WritePixelColor(static_cast<int>(projtl.position.GetX()),
                         canvas.GetHeight() - static_cast<int>(projtl.position.GetY()),
                         ray_tracer::Color{1, 0, 0});
  canvas.PrintPixels();
  canvas.CanvasToPPM("/tmp/canvas.ppm");

  return 0;
}
