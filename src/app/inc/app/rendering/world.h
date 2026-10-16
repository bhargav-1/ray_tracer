#pragma once
#include <vector>
#include "app/materials/color.h"
#include "app/shapes/sphere.h"
#include "app/shapes/shape.h"
#include "app/rendering/point_light.h"
#include <memory>

namespace ray_tracer {
class World {
 public:
  World() = default;
  void AddObject(std::shared_ptr<Shape> object) { objects_.push_back(std::move(object)); }
  void UpdateObjects(std::size_t index, std::shared_ptr<Shape> object) { objects_.at(index) = std::move(object); }
  void AddLight(const PointLight& light) { lights_.push_back(light); }
  void ChangeDefaultLight(const PointLight& light) {
    if (lights_.empty()) {
      lights_.push_back(light);
    }else {
      lights_[0] = light;
    }
  }
  [[nodiscard]] auto Objects() const -> const std::vector<std::shared_ptr<Shape>>& { return objects_; }
  [[nodiscard]] auto Lights() const -> const std::vector<PointLight>& { return lights_; }
  static auto DefaultWorld() -> World {
   World world;
    world.AddLight(PointLight(Point(-10, 10, -10), Color(1, 1, 1)));
    auto sphere1 = std::make_shared<ray_tracer::Sphere>(1);
    auto sphere1_material = sphere1->GetMaterial();
    sphere1_material.SetColor(Color(0.8, 1.0, 0.6));
    sphere1_material.SetDiffuse(0.7);
    sphere1_material.SetSpecular(0.2);
    sphere1->SetMaterial(sphere1_material);
    auto sphere2 = std::make_shared<ray_tracer::Sphere>(2);
    sphere2->SetTransform(ray_tracer::Matrix<4,4>::Scaling(0.5, 0.5, 0.5));
    world.AddObject(sphere1);
    world.AddObject(sphere2);
    return world;
  }

 private:
  std::vector<PointLight> lights_;
  std::vector<std::shared_ptr<Shape>> objects_;
};
}  // namespace ray_tracer
