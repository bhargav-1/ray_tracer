#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "computations.h"
#include "intersection.h"
#include "lighting.h"
#include "point.h"
#include "point_light.h"
#include "ray.h"
#include "render_rows.h"
#include "shape.h"
#include "sphere.h"
#include "utilities.h"
#include "vec3.h"
#include "world.h"

namespace ray_tracer {

inline auto Intersect(const Ray& ray, const World& world) -> std::vector<Intersection> {
  std::vector<Intersection> intersections;
  intersections.reserve(world.Objects().size() * 2);
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

inline auto PrepareComputations(const Intersection& intersection,
                                const Ray& ray,
                                std::vector<Intersection> intersections_xs = {}) -> Computations {
  std::vector<const Shape*> containers{};
  double refractive_index_n1 = 1.0;
  double refractive_index_n2 = 1.0;
  if (intersections_xs.empty()) {
    intersections_xs = std::vector<Intersection>{intersection};
  }
  for (auto const& each_intersection : intersections_xs) {
    if (each_intersection == intersection) {
      if (containers.empty()) {
        refractive_index_n1 = 1.0;
      } else {
        refractive_index_n1 = containers.back()->GetMaterial().GetRefractiveIndex();
      }
    }
    if (std::ranges::find(containers, each_intersection.GetObject()) != containers.end()) {
      std::erase(containers, each_intersection.GetObject());
    } else {
      const auto* object = each_intersection.GetObject();
      containers.push_back(object);
    }

    if (each_intersection == intersection) {
      if (containers.empty()) {
        refractive_index_n2 = 1;
      } else {
        refractive_index_n2 = containers.back()->GetMaterial().GetRefractiveIndex();
      }
      break;
    }
  }
  const auto time = intersection.GetTime();
  const auto* object = intersection.GetObject();
  const auto point = ray.Position(time);
  const auto eye_v = -ray.GetDirection();
  auto normal_v = object->NormalAt(point, intersection);
  bool inside = false;
  if (normal_v.Dot(eye_v) < 0) {
    inside = true;
    normal_v = -normal_v;
  }
  const auto over_point = point + normal_v * utilities::kEpsilon;
  const auto under_point = point - normal_v * utilities::kEpsilon;
  const auto reflect_v = ray.GetDirection().Reflect(normal_v);
  return Computations{time,
                      object,
                      point,
                      eye_v,
                      normal_v,
                      inside,
                      over_point,
                      reflect_v,
                      refractive_index_n1,
                      refractive_index_n2,
                      under_point};
}

inline auto IsShadowed(const World& world, const Point& point, const PointLight& light) -> bool {
  const Vec3 vector_to_light = light.GetPosition() - point;
  const double distance_to_light = vector_to_light.Magnitude();
  const Ray shadow_ray(point, vector_to_light.Normalize());
  return std::ranges::any_of(world.Objects(), [&](const std::shared_ptr<Shape>& object) {
    return object->Occludes(shadow_ray, distance_to_light);
  });
}

auto GetReflectedColor(const World& world, const Computations& computations, int remaining = 5) -> Color;
auto GetRefractedColor(const World& world, const Computations& computations, int remaining = 5) -> Color;
inline auto Schlick(const Computations& computations) -> double;
// NOLINTNEXTLINE(misc-no-recursion)
inline auto ShadeHit(const World& world, const Computations& computations, const int remaining = 5)
    -> Color {  // NOLINT(misc-no-recursion)
  Color color{0, 0, 0};
  Color reflected_color{0, 0, 0};
  Color refracted_color{0, 0, 0};

  for (const auto& world_light : world.Lights()) {
    const auto shadowed = IsShadowed(world, computations.GetOverPoint(), world_light);
    color = color + Lighting(computations.GetObject()->GetMaterial(),
                             *computations.GetObject(),
                             world_light,
                             computations.GetPoint(),
                             computations.GetEyeV(),
                             computations.GetNormalV(),
                             shadowed);
  }
  reflected_color = GetReflectedColor(world, computations, remaining);
  refracted_color = GetRefractedColor(world, computations, remaining);
  if (auto material = computations.GetObject()->GetMaterial();
      material.GetReflective() > 0 && material.GetTransparency() > 0) {
    auto reflectance = Schlick(computations);
    color = color + reflected_color * reflectance + refracted_color * (1 - reflectance);
      } else {
        color = color + reflected_color + refracted_color;
      }
  return color;
}

// NOLINTNEXTLINE(misc-no-recursion)
inline auto ColorAt(const World& world, const Ray& ray, const int remaining = 5)
    -> Color {  // NOLINT(misc-no-recursion)
  auto intersections = Intersect(ray, world);
  auto hit = Hit(intersections);
  if (!hit.has_value()) {
    return Color{0, 0, 0};
  }
  return ShadeHit(world, PrepareComputations(hit.value(), ray, intersections), remaining);
}

// NOLINTNEXTLINE(misc-no-recursion)
inline auto GetReflectedColor(const World& world, const Computations& computations, const int remaining)
    -> Color {  // NOLINT(misc-no-recursion)
  if (computations.GetObject()->GetMaterial().GetReflective() == 0) {
    return Color{0, 0, 0};
  }
  if (remaining <= 0) {
    return Color{0, 0, 0};
  }
  const auto reflect_ray = Ray(computations.GetOverPoint(), computations.GetReflectV());
  const auto color = ColorAt(world, reflect_ray, remaining - 1);
  return color * computations.GetObject()->GetMaterial().GetReflective();
}

// NOLINTNEXTLINE(misc-no-recursion)
inline auto GetRefractedColor(const World& world, const Computations& computations, int remaining)
    -> Color {                                                           // NOLINT(misc-no-recursion)
  if (computations.GetObject()->GetMaterial().GetTransparency() == 0) {  // NOLINT
    return Color{0, 0, 0};
  }
  if (remaining <= 0) {
    return Color{0, 0, 0};
  }
  // sin (theta_i) / sin(theta_t) = n2/n1
  auto n_ratio = computations.GetRefractiveIndexN1() / computations.GetRefractiveIndexN2();
  auto cos_theta_i = computations.GetNormalV().Dot(computations.GetEyeV());
  auto sin_theta_i = std::sqrt(1 - cos_theta_i * cos_theta_i);
  auto sin_theta_t = n_ratio * sin_theta_i;

  // total interal reflection
  if (sin_theta_t * sin_theta_t >= 1) {
    return Color{0, 0, 0};
  }
  auto cos_theta_t = std::sqrt(1 - sin_theta_t * sin_theta_t);
  auto direction = computations.GetNormalV() * (n_ratio * cos_theta_i - cos_theta_t) - computations.GetEyeV() * n_ratio;
  auto refracted_ray = Ray(computations.GetUnderPoint(), direction);
  auto color = ColorAt(world, refracted_ray, remaining - 1) * computations.GetObject()->GetMaterial().GetTransparency();
  return color;
}

inline auto Schlick(const Computations& computations) -> double {
  auto cos = computations.GetEyeV().Dot(computations.GetNormalV());
  if (computations.GetRefractiveIndexN1() > computations.GetRefractiveIndexN2()) {
    auto n_ratio = computations.GetRefractiveIndexN1() / computations.GetRefractiveIndexN2();
    auto sin2_t = n_ratio * n_ratio * (1 - cos * cos);
    if (sin2_t > 1) {
      return 1.0;
    }
    auto cos_t = std::sqrt(1 - sin2_t);
    cos = cos_t;
  }
  const auto reflectance_at_normal = std::pow(
      (computations.GetRefractiveIndexN1() - computations.GetRefractiveIndexN2()) /
          (computations.GetRefractiveIndexN1() + computations.GetRefractiveIndexN2()),
      2);
  //https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
  // https://www.flipcode.com/archives/reflection_transmission.pdf
  return reflectance_at_normal + (1 - reflectance_at_normal) * std::pow(1 - cos, 5);
}

inline auto Render(Camera& camera, const World& world) -> Canvas {
  auto image = Canvas(camera.GetHorizontalSize(), camera.GetVerticalSize());
  int last_bucket = -1;
  int completed_rows = 0;
  std::mutex progress_mutex;
  for (const auto& object : world.Objects()) {
    (void)object->BoundsOf();
  }
  auto start = std::chrono::high_resolution_clock::now();
  ParallelRenderRows(camera.GetVerticalSize(), [&](const int y_size) {
    for (auto x_size = 0; x_size < camera.GetHorizontalSize(); x_size++) {
      auto ray = camera.GetRayForPixel(x_size, y_size);
      auto color = ColorAt(world, ray);
      image.WritePixelColor(x_size, y_size, color);
    }
    const std::lock_guard lock{progress_mutex};
    const int percent = (++completed_rows * 100) / camera.GetVerticalSize();
    if (const int bucket = percent / 10; bucket != last_bucket) {
      last_bucket = bucket;
      std::cout << "Render progress: " << bucket * 10 << "%\n";
    }
    return true;
  });
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

}  // namespace ray_tracer
