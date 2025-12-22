#pragma once
#include <vector>
#include "color.h"
#include "point_light.h"

namespace ray_tracer {
class World {
 public:
  World() = default;
  void AddObject(const Sphere& object) { objects_.push_back(object); }
  void AddLight(const PointLight& light) { lights_.push_back(light); }
  [[nodiscard]] auto Objects() const -> const std::vector<Sphere>& { return objects_; }
  [[nodiscard]] auto Lights() const -> const std::vector<PointLight>& { return lights_; }
  static auto DefaultWorld() -> World {
   World world;
    world.AddLight(PointLight(Point(-10, 10, -10), Color(1, 1, 1)));
    auto sphere1 = ray_tracer::Sphere(1);
    auto sphere2 = ray_tracer::Sphere(2);
    sphere2.SetTransform(ray_tracer::Matrix<4,4>::Scaling(0.5, 0.5, 0.5));
    world.AddObject(sphere1);
    world.AddObject(sphere2);
    return world;
  }

 private:
  std::vector<PointLight> lights_;
  Color light_color_;
  std::vector<Sphere> objects_;
};
}  // namespace ray_tracer
