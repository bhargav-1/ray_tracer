#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <vector>

#include "intersection.h"
#include "point.h"
#include "ray.h"
#include "sphere.h"
#include "shape.h"
#include "utilities.h"
#include "vec3.h"
#include "world.h"
#include "computations.h"
#include "lighting.h"
#include "camera.h"
#include "canvas.h"

namespace ray_tracer {

inline auto Intersect(const Ray& ray, const World& world) -> std::vector<Intersection> {
  std::vector<Intersection> intersections;
  for (auto const& object : world.Objects()) {
    auto object_intersections = object->Intersect(ray);
    // std::cout << " object_intersections size: " << object_intersections.size() << "\n";
    intersections.insert(intersections.end(), object_intersections.begin(), object_intersections.end());
  }
  std::ranges::sort(intersections, {}, &Intersection::GetTime);
  return intersections;
}


inline auto Hit(const std::vector<Intersection>& intersections) -> std::optional<Intersection> {
  auto current_intersection = std::optional<Intersection>();
  for (const auto& intersection : intersections) {
    if (intersection.GetTime() >= 0) {
      if (!current_intersection.has_value()) {
        current_intersection = std::optional<Intersection>(intersection);
      } else {
        if (intersection.GetTime() < current_intersection.value().GetTime()) {
          current_intersection = intersection;
        }
      }
    }
  }
  return current_intersection;
}

inline auto PrepareComputations(const Intersection& intersection, const Ray& ray) -> Computations {
  const auto time = intersection.GetTime();
  const auto* object = intersection.GetObject();
  const auto point = ray.Position(time);
  const auto eye_v = -ray.GetDirection();
  auto normal_v = object->NormalAt(point);
  bool inside = false;
  if (normal_v.Dot(eye_v) < 0) {
    inside = true;
    normal_v = -normal_v;
  }
  const auto over_point = point + normal_v * utilities::kEpsilon;
  return Computations{time, object, point, eye_v, normal_v, inside, over_point};
}

inline auto IsShadowed(const World& world, const Point& point) -> bool {
  for (const auto& light : world.Lights()) {
    Vec3 const vector_to_light = light.GetPosition() - point;
    double const distance_to_light = vector_to_light.Magnitude();
    Vec3 const direction_to_light = vector_to_light.Normalize();
    Ray const shadow_ray(point, direction_to_light);
    auto intersections = Intersect(shadow_ray, world);
    if (auto hit = Hit(intersections); hit.has_value() && hit.value().GetTime() < distance_to_light) {
      return true;
    }
  }
  return false;
}

inline auto ShadeHit(const World& world, const Computations& computations) -> Color {
  Color color{0, 0, 0};
  for (auto world_light : world.Lights()) {
    color = color + Lighting(computations.GetObject()->GetMaterial(),
                             world_light,
                             computations.GetPoint(),
                             computations.GetEyeV(),
                             computations.GetNormalV(),
                             IsShadowed(world, computations.GetOverPoint()));
  }
  return color;
}

inline auto ColorAt(const World& world, const Ray& ray) -> Color {
  auto intersections = Intersect(ray, world);
  auto hit = Hit(intersections);
  if (!hit.has_value()) {
    return Color{0, 0, 0};
  }
  return ShadeHit(world, PrepareComputations(hit.value(), ray));
}



inline auto Render(Camera& camera, const World& world) -> Canvas {
  auto image = Canvas(camera.GetHorizontalSize(), camera.GetVerticalSize());
  int last_bucket = -1;
  auto start = std::chrono::high_resolution_clock::now();
  for (auto y_size = 0; y_size < camera.GetVerticalSize(); y_size++) {
    for (auto x_size = 0; x_size < camera.GetHorizontalSize(); x_size++) {
      auto ray = camera.GetRayForPixel(x_size, y_size);
      auto color = ColorAt(world, ray);
      image.WritePixelColor(x_size, y_size, color);
    }
    const int percent = ((y_size + 1) * 100) / camera.GetVerticalSize();
    if (const int bucket = percent / 10; bucket != last_bucket) {
      last_bucket = bucket;
      std::cout << "Render progress: " << bucket * 10 << "%\n";
    }
  }
  const auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Writing Pixel time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << "ms\n";

  std::cout << "Writing Pixels to file\n";
  auto start_disk_write = std::chrono::high_resolution_clock::now();
  image.CanvasToPPM("/tmp/chapter7.ppm");
  auto end_disk_write = std::chrono::high_resolution_clock::now();
  std::cout << "Writing Pixels to file time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_disk_write - start_disk_write).count()
            << "ms\n";
  return image;
}

}
