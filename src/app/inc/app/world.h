#pragma once
#include <vector>
#include "color.h"
#include "point_light.h"

namespace ray_tracer {
class World {
 public:
  constexpr World() = default;
  void constexpr AddObject(const Sphere& object) { objects_.push_back(object); }
  void constexpr AddLight(const PointLight& light) { lights_.push_back(light); }
  void constexpr ChangeDefaultLight(const PointLight& light) {
    if (lights_.empty()) {
      lights_.push_back(light);
    }else {
      lights_[0] = light;
    }
  }
  [[nodiscard]] auto constexpr Objects() const -> const std::vector<Sphere>& { return objects_; }
  [[nodiscard]] auto constexpr Lights() const -> const std::vector<PointLight>& { return lights_; }
  static auto DefaultWorld() -> World {
   World world;
    world.AddLight(PointLight(Point(-10, 10, -10), Color(1, 1, 1)));
    auto sphere1 = ray_tracer::Sphere(1);
    auto sphere1_material = sphere1.GetMaterial();
    sphere1_material.SetColor(Color(0.8, 1.0, 0.6));
    sphere1_material.SetDiffuse(0.7);
    sphere1_material.SetSpecular(0.2);
    sphere1.SetMaterial(sphere1_material);
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
