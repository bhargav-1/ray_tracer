#include "app/scene/scene_compiler.h"

#include <cstddef>
#include <cmath>
#include <exception>
#include <memory>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/camera.h"
#include "app/color.h"
#include "app/group.h"
#include "app/material.h"
#include "app/matrix.h"
#include "app/pattern.h"
#include "app/point.h"
#include "app/point_light.h"
#include "app/scene/scene.h"
#include "app/shape_catalog.h"
#include "app/transformations.h"
#include "app/vec3.h"
#include "app/world.h"

namespace ray_tracer::scene_runtime {
namespace {

auto ToColor(const scene::ColorRgb& color) -> ray_tracer::Color {
  return {color.r, color.g, color.b};
}

auto ToPoint(const scene::Vec3& value) -> ray_tracer::Point {
  return {value.x, value.y, value.z};
}

auto ToVec3(const scene::Vec3& value) -> ray_tracer::Vec3 {
  return {value.x, value.y, value.z};
}

auto HasFiniteBounds(const ray_tracer::Shape& shape) -> bool {
  const auto bounds = shape.ParentSpaceBounds();
  const auto min = bounds.GetMin();
  const auto max = bounds.GetMax();
  return std::isfinite(min.GetX()) && std::isfinite(min.GetY()) && std::isfinite(min.GetZ()) &&
         std::isfinite(max.GetX()) && std::isfinite(max.GetY()) && std::isfinite(max.GetZ());
}

auto BuildTransformMatrix(const scene::Transform& transform) -> ray_tracer::Matrix<4, 4> {
  auto matrix = ray_tracer::Matrix<4, 4>::Translation(
      transform.translate.x, transform.translate.y, transform.translate.z);
  constexpr auto kDegToRad = std::numbers::pi / 180.0;
  if (transform.rotate_deg.x != 0.0F) {
    matrix = matrix * ray_tracer::Matrix<4, 4>::RotationX(transform.rotate_deg.x * kDegToRad);
  }
  if (transform.rotate_deg.y != 0.0F) {
    matrix = matrix * ray_tracer::Matrix<4, 4>::RotationY(transform.rotate_deg.y * kDegToRad);
  }
  if (transform.rotate_deg.z != 0.0F) {
    matrix = matrix * ray_tracer::Matrix<4, 4>::RotationZ(transform.rotate_deg.z * kDegToRad);
  }
  return matrix * ray_tracer::Matrix<4, 4>::Scaling(transform.scale.x, transform.scale.y, transform.scale.z);
}

auto SolidChildColor(const scene::Pattern& pattern, const std::string& label) -> ray_tracer::Color {
  if (pattern.type != scene::PatternType::kSolid) {
    throw std::runtime_error(label + " requires solid child patterns in the current runtime");
  }
  return ToColor(pattern.color);
}

auto CompilePattern(const scene::Pattern& pattern) -> std::shared_ptr<ray_tracer::Pattern> {  // NOLINT(misc-no-recursion)
  std::shared_ptr<ray_tracer::Pattern> compiled;
  switch (pattern.type) {
    case scene::PatternType::kSolid:
      compiled = std::make_shared<ray_tracer::SolidPattern>(ToColor(pattern.color));
      break;
    case scene::PatternType::kStripe:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("stripe pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::StripePattern>(
          CompilePattern(pattern.children[0]), CompilePattern(pattern.children[1]));
      break;
    case scene::PatternType::kRing:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("ring pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::RingPattern>(
          CompilePattern(pattern.children[0]), CompilePattern(pattern.children[1]));
      break;
    case scene::PatternType::kCheckers:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("checkers pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::CheckersPattern>(
          CompilePattern(pattern.children[0]), CompilePattern(pattern.children[1]));
      break;
    case scene::PatternType::kBlended:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("blended pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::BlendedPattern>(
          CompilePattern(pattern.children[0]), CompilePattern(pattern.children[1]));
      break;
    case scene::PatternType::kUvCheckers:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("uv_checkers pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::UVCheckersPattern>(
          pattern.width, pattern.height, CompilePattern(pattern.children[0]), CompilePattern(pattern.children[1]));
      break;
    case scene::PatternType::kPerturbed:
      if (pattern.children.size() != 1) {
        throw std::runtime_error("perturbed pattern requires exactly 1 source pattern");
      }
      compiled = std::make_shared<ray_tracer::PerturbedPattern>(
          CompilePattern(pattern.children[0]), pattern.perturb_scale);
      break;
    case scene::PatternType::kGradient:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("gradient pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::GradientPattern>(
          SolidChildColor(pattern.children[0], "gradient"), SolidChildColor(pattern.children[1], "gradient"));
      break;
    case scene::PatternType::kRadialGradient:
      if (pattern.children.size() != 2) {
        throw std::runtime_error("radial_gradient pattern requires exactly 2 child patterns");
      }
      compiled = std::make_shared<ray_tracer::RadialGradientPattern>(
          SolidChildColor(pattern.children[0], "radial_gradient"),
          SolidChildColor(pattern.children[1], "radial_gradient"));
      break;
  }
  compiled->SetPatternTransform(BuildTransformMatrix(pattern.transform));
  return compiled;
}

auto CompileMaterial(const scene::Material& material) -> ray_tracer::Material {
  ray_tracer::Material compiled{
      ToColor(material.color),
      material.ambient,
      material.diffuse,
      material.specular,
      material.shininess,
      material.reflective,
      material.transparency,
      material.refractive_index};
  if (material.pattern.has_value()) {
    compiled.SetPattern(CompilePattern(*material.pattern));
  }
  return compiled;
}

auto CompileObject(const scene::Object& object, std::size_t& next_id)  // NOLINT(misc-no-recursion)
    -> std::shared_ptr<ray_tracer::Shape> {
  if (object.IsGroup()) {
    auto group = std::make_shared<ray_tracer::Group>(next_id++);
    group->SetTransform(BuildTransformMatrix(object.transform));
    for (const auto& child : object.Group().children) {
      if (child.visible) {
        group->AddChild(CompileObject(child, next_id));
      }
    }
    return group;
  }

  const auto& shape_spec = object.Shape();
  const auto& registration = shape_catalog::Find(shape_spec.type);
  if (registration.supports_axial_bounds &&
      shape_spec.axial_bounds.minimum >= shape_spec.axial_bounds.maximum) {
    throw std::runtime_error("object '" + object.name + "' requires minimum bounds below maximum bounds");
  }
  auto shape = registration.create(next_id++, shape_spec.axial_bounds);
  shape->SetTransform(BuildTransformMatrix(object.transform));
  shape->SetCastShadow(shape_spec.cast_shadow);
  shape->SetMaterial(CompileMaterial(shape_spec.material));
  return shape;
}

}  // namespace

auto CompileScene(const scene::Scene& scene) -> CompileSceneResult {
  try {
    if (!scene.camera.has_value()) {
      throw std::runtime_error("scene requires a camera");
    }
    ray_tracer::World world;
    for (const auto& light : scene.lights) {
      if (!light.visible) {
        continue;
      }
      world.AddLight(ray_tracer::PointLight(ToPoint(light.position), ToColor(light.color)));
    }
    std::size_t next_id = 1;
    constexpr int kBvhThreshold = 4;
    std::vector<std::shared_ptr<ray_tracer::Shape>> objects;
    std::vector<std::shared_ptr<ray_tracer::Shape>> finite_objects;
    for (const auto& object : scene.objects) {
      if (!object.visible) {
        continue;
      }
      auto compiled_object = CompileObject(object, next_id);
      if (HasFiniteBounds(*compiled_object)) {
        finite_objects.push_back(compiled_object);
      }
      objects.push_back(std::move(compiled_object));
    }
    if (finite_objects.size() >= static_cast<std::size_t>(kBvhThreshold)) {
      auto root = std::make_shared<ray_tracer::Group>(next_id++);
      for (const auto& object : finite_objects) {
        root->AddChild(object);
      }
      root->Divide(kBvhThreshold);
      world.AddObject(root);
      for (const auto& object : objects) {
        if (!HasFiniteBounds(*object)) {
          world.AddObject(object);
        }
      }
    } else {
      for (const auto& object : objects) {
        if (HasFiniteBounds(*object)) {
          object->Divide(kBvhThreshold);
        }
        world.AddObject(object);
      }
    }

    // Populate the group caches after subdivision, before any render rays are traced.
    for (const auto& object : world.Objects()) {
      (void)object->BoundsOf();
    }

    const auto& camera_spec = *scene.camera;
    ray_tracer::Camera camera{
        camera_spec.resolution.width,
        camera_spec.resolution.height,
        camera_spec.fov_deg * std::numbers::pi / 180.0};
    camera.SetTransform(ray_tracer::ViewTransform(
        ToPoint(camera_spec.from), ToPoint(camera_spec.to), ToVec3(camera_spec.up)));
    return {.ok = true, .compiled = CompiledScene{.world = std::move(world), .camera = camera}, .error = {}};
  } catch (const std::exception& exception) {
    return {.ok = false, .compiled = std::nullopt, .error = exception.what()};
  }
}

}  // namespace ray_tracer::scene_runtime
