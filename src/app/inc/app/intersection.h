#pragma once
#include <algorithm>
#include <optional>
#include <vector>
#include "app/lighting.h"
#include "app/ray.h"
#include "app/sphere.h"
#include "app/world.h"
#include "camera.h"
#include "canvas.h"

namespace ray_tracer {
class Intersection {
 public:
  constexpr Intersection(double time, Sphere sphere) : time_(time), sphere_(sphere) {};
  auto friend operator==(Intersection const& lhs, Intersection const& rhs) -> bool;
  [[nodiscard]] constexpr auto GetTime() const { return time_; }
  [[nodiscard]] auto constexpr GetObject() const { return sphere_; }

 private:
  double time_;
  Sphere sphere_;
};

inline auto operator==(Intersection const& lhs, Intersection const& rhs) -> bool {
  return lhs.time_ == rhs.time_ && lhs.sphere_ == rhs.sphere_;
}

// can be constexpr in later cpp23++or cpp20++
//en.wikipedia.org/wiki/Line–sphere_intersection
inline auto Intersect(const Ray& ray, const Sphere& sphere) -> std::vector<Intersection> {
  // transform the ray to the sphere's coordinate system
  auto transform = sphere.GetTransform().Inverse();
  auto transformed_ray = ray.Transform(transform);

  // following is just the algorithm for ray/spher intersection
  const ray_tracer::Vec3 sphere_to_ray = transformed_ray.GetOrigin() - Point(0, 0, 0);
  const auto a_var = transformed_ray.GetDirection().Dot(transformed_ray.GetDirection());
  const auto b_var = 2 * transformed_ray.GetDirection().Dot(sphere_to_ray);
  const auto c_var = sphere_to_ray.Dot(sphere_to_ray) - 1;
  auto discriminant = (b_var * b_var) - (4 * a_var * c_var);
  if (discriminant < 0) {
    return {};
  }
  // this t is time/unit in the ray
  auto t1_var = (-b_var - std::sqrt(discriminant)) / (2 * a_var);
  auto t2_var = (-b_var + std::sqrt(discriminant)) / (2 * a_var);
  return {Intersection(t1_var, sphere), Intersection(t2_var, sphere)};
}

inline auto Intersect(const Ray& ray, const World& world) -> std::vector<Intersection> {
  std::vector<Intersection> intersections;
  for (auto const& object : world.Objects()) {
    auto object_intersections = Intersect(ray, object);
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

class Computations {
 public:
  constexpr Computations(double time, Sphere object, Point point, Vec3 eye_v, Vec3 normal_v, bool inside)
      : time_(time),
        object_(object),
        point_(std::move(point)),
        eye_v_(std::move(eye_v)),
        normal_v_(std::move(normal_v)),
        inside_(inside) {};
  [[nodiscard]] constexpr auto GetTime() const -> double { return time_; }
  [[nodiscard]] constexpr auto GetObject() const -> Sphere { return object_; }
  [[nodiscard]] constexpr auto GetPoint() const -> Point { return point_; }
  [[nodiscard]] constexpr auto GetEyeV() const -> Vec3 { return eye_v_; }
  [[nodiscard]] constexpr auto GetNormalV() const -> Vec3 { return normal_v_; }
  [[nodiscard]] constexpr auto IsInside() const -> bool { return inside_; }

 private:
  double time_;
  Sphere object_;
  Point point_;
  Vec3 eye_v_;
  Vec3 normal_v_;
  bool inside_;
};

inline auto PrepareComputations(const Intersection& intersection, const Ray& ray) -> Computations {
  const auto time = intersection.GetTime();
  const auto object = intersection.GetObject();
  const auto point = ray.Position(time);
  const auto eye_v = -ray.GetDirection();
  auto normal_v = object.NormalAt(point);
  bool inside = false;
  if (normal_v.Dot(eye_v) < 0) {
    inside = true;
    normal_v = -normal_v;
  }
  return Computations{time, object, point, eye_v, normal_v, inside};
}
inline auto ShadeHit(const World& world, const Computations& computations) -> Color {
  Color color{0, 0, 0 };
  for (auto world_light : world.Lights()) {
    color = color + Lighting(computations.GetObject().GetMaterial(),
                             world_light,
                             computations.GetPoint(),
                             computations.GetEyeV(),
                             computations.GetNormalV());
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

inline auto Render( Camera& camera, const World& world) -> Canvas {
auto image = Canvas(camera.GetHorizontalSize(), camera.GetVerticalSize()) ;
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
  std::cout << "Writing Pixel time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";

  std::cout << "Writing Pixels to file\n";
  auto start_disk_write = std::chrono::high_resolution_clock::now();
  image.CanvasToPPM("/tmp/chapter7.ppm");
  auto end_disk_write = std::chrono::high_resolution_clock::now();
  std::cout << "Writing Pixels to file time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_disk_write - start_disk_write).count() << "ms\n";
  return image;
}
}  // namespace ray_tracer
