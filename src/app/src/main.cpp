#include <iostream>
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

void Tick(const Environment& env, Projectile& proj) {
  proj.velocity = proj.velocity + env.gravity + env.wind;
  proj.position = proj.position + proj.velocity;
  std::cout << "x: " << proj.position.GetX() << ", y: " << proj.position.GetY() << ", z: " << proj.position.GetZ()
            << '\n';
}

auto main(int argc, char* argv[]) -> int {
  Projectile p{ray_tracer::Point{0, 1, 0}, ray_tracer::Vec3{1, 1, 0}.Normalize()};
  Environment e{ray_tracer::Vec3{0, -0.1, 0}, ray_tracer::Vec3{-0.01, 0, 0}};
  int count = 0;
  std::cout << "x: " << p.position.GetX() << ", y: " << p.position.GetY() << ", z: " << p.position.GetZ() << '\n';
  while (p.position.GetY() >= -1) {
    Tick(e, p);
    std::cout << " count : " << count << '\n';
    std::cout << "x: " << p.position.GetX() << ", y: " << p.position.GetY() << ", z: " << p.position.GetZ() << '\n';
    std::cout << "-  - - -- - - - -- -- - -- - - - - - -- -" << '\n';
    count++;
  }
  return 0;
};