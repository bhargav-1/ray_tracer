#pragma once
#include "material.h"
#include "pattern.h"
#include "point.h"
#include "point_light.h"
#include "shape.h"
#include "vec3.h"

namespace ray_tracer {

inline auto ColorAtObject(const Pattern& pattern, const Shape& object, const Point& point) -> Color {
  const auto object_point = object.GetTransform().Inverse() * point;
  const auto pattern_point = pattern.GetPatternTransform().Inverse() * object_point;
  return pattern.ColorAt(pattern_point);
}

auto inline Lighting(const Material& material,
                     const Shape& shape,
                     const PointLight& light,
                     const Point& position,
                     const Vec3& eyev,
                     const Vec3& normalv,
                     bool in_shadow = false) -> Color {
  Color color;
  if (const auto* pattern = material.GetPattern()) {
    color = ColorAtObject(*pattern, shape, position);
  } else {
    color = material.GetColor();
  }
  const auto effective_color = color * light.GetIntensity();
  const auto lightv = Vec3(light.GetPosition() - position).Normalize();
  const auto ambient = effective_color * material.GetAmbient();
  const auto light_dot_normal = lightv.Dot(normalv);
  Color diffuse;
  Color specular;
  if (light_dot_normal < 0 || in_shadow) {
    diffuse = Color(0, 0, 0);
    specular = Color(0, 0, 0);
  } else {
    diffuse = effective_color * material.GetDiffuse() * light_dot_normal;
    const auto reflectv = Vec3(-lightv.Reflect(normalv));
    if (const auto reflect_dot_eye = reflectv.Dot(eyev); reflect_dot_eye <= 0) {
      specular = Color(0, 0, 0);
    } else {
      const auto factor = pow(reflect_dot_eye, material.GetShininess());
      specular = light.GetIntensity() * material.GetSpecular() * factor;
    }
  }
  return ambient + diffuse + specular;
}
}  // namespace ray_tracer
