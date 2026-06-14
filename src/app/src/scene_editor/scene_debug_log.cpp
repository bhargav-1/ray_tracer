#include "app/scene_editor/scene_debug_log.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "app/camera.h"
#include "app/material.h"
#include "app/plane.h"
#include "app/point.h"
#include "app/scene/scene.h"
#include "app/scene/scene_compiler.h"
#include "app/shape.h"
#include "app/sphere.h"

namespace ray_tracer::scene_editor {
namespace {

auto ObjectTypeName(scene::ObjectType type) -> const char* {
  switch (type) {
    case scene::ObjectType::kSphere:
      return "sphere";
    case scene::ObjectType::kPlane:
      return "plane";
  }
  return "unknown";
}

auto RuntimeShapeTypeName(const Shape& shape) -> const char* {
  if (dynamic_cast<const Sphere*>(&shape) != nullptr) {
    return "sphere";
  }
  if (dynamic_cast<const Plane*>(&shape) != nullptr) {
    return "plane";
  }
  return "unknown";
}

auto MixFingerprint(std::uint64_t value, std::uint64_t input) -> std::uint64_t {
  value ^= input + 0x9e3779b97f4a7c15ULL + (value << 6U) + (value >> 2U);
  return value;
}

auto Quantize(float value) -> std::uint64_t {
  return static_cast<std::uint64_t>(static_cast<std::int64_t>(std::llround(value * 10000.0F)));
}

auto MixVec3(std::uint64_t value, const scene::Vec3& vec) -> std::uint64_t {
  value = MixFingerprint(value, Quantize(vec.x));
  value = MixFingerprint(value, Quantize(vec.y));
  return MixFingerprint(value, Quantize(vec.z));
}

auto MixColor(std::uint64_t value, const scene::ColorRgb& color) -> std::uint64_t {
  value = MixFingerprint(value, Quantize(color.r));
  value = MixFingerprint(value, Quantize(color.g));
  return MixFingerprint(value, Quantize(color.b));
}

auto MixTransform(std::uint64_t value, const scene::Transform& transform) -> std::uint64_t {
  value = MixVec3(value, transform.translate);
  value = MixVec3(value, transform.rotate_deg);
  return MixVec3(value, transform.scale);
}

}  // namespace

auto CountVisibleObjects(const scene::Scene& scene) -> std::size_t {
  return static_cast<std::size_t>(std::ranges::count_if(scene.objects, [](const scene::Object& object) {
    return object.visible;
  }));
}

auto LogAuthoringSceneSnapshot(const char* mode, const scene::Scene& scene) -> void {
  std::cerr << "[scene_editor] authoring snapshot mode=" << mode
            << " lights=" << scene.lights.size()
            << " objects=" << scene.objects.size()
            << " visible=" << CountVisibleObjects(scene) << '\n';
  for (std::size_t index = 0; index < scene.lights.size(); ++index) {
    const auto& light = scene.lights[index];
    std::cerr << std::fixed << std::setprecision(3)
              << "[scene_editor] authoring light mode=" << mode
              << " index=" << index
              << " position=[" << light.position.x << ' ' << light.position.y << ' ' << light.position.z << ']'
              << " color=[" << light.color.r << ' ' << light.color.g << ' ' << light.color.b << "]\n";
  }
  for (std::size_t index = 0; index < scene.objects.size(); ++index) {
    const auto& object = scene.objects[index];
    const auto& material = object.material;
    std::cerr << std::fixed << std::setprecision(3)
              << "[scene_editor] authoring object mode=" << mode
              << " index=" << index
              << " id=" << index + 1
              << " name=\"" << object.name << '"'
              << " type=" << ObjectTypeName(object.type)
              << " visible=" << (object.visible ? 1 : 0)
              << " cast_shadow=" << (object.cast_shadow ? 1 : 0)
              << " translate=[" << object.transform.translate.x << ' ' << object.transform.translate.y << ' '
              << object.transform.translate.z << ']'
              << " rotate_deg=[" << object.transform.rotate_deg.x << ' ' << object.transform.rotate_deg.y << ' '
              << object.transform.rotate_deg.z << ']'
              << " scale=[" << object.transform.scale.x << ' ' << object.transform.scale.y << ' '
              << object.transform.scale.z << ']'
              << " color=[" << material.color.r << ' ' << material.color.g << ' ' << material.color.b << ']'
              << " ambient=" << material.ambient
              << " diffuse=" << material.diffuse
              << " specular=" << material.specular
              << " shininess=" << material.shininess
              << " reflective=" << material.reflective
              << " transparency=" << material.transparency
              << " refractive_index=" << material.refractive_index
              << " pattern=" << (material.pattern.has_value() ? 1 : 0) << '\n';
  }
}

auto LogRuntimeSceneSnapshot(const char* mode,
                             int render_id,
                             const scene_runtime::CompiledScene& compiled_scene) -> void {
  std::cerr << "[scene_editor] runtime snapshot mode=" << mode
            << " id=" << render_id
            << " lights=" << compiled_scene.world.Lights().size()
            << " objects=" << compiled_scene.world.Objects().size()
            << " camera=" << compiled_scene.camera.GetHorizontalSize() << 'x' << compiled_scene.camera.GetVerticalSize()
            << '\n';
  for (std::size_t index = 0; index < compiled_scene.world.Lights().size(); ++index) {
    const auto& light = compiled_scene.world.Lights()[index];
    const auto& position = light.GetPosition();
    const auto& intensity = light.GetIntensity();
    std::cerr << std::fixed << std::setprecision(3)
              << "[scene_editor] runtime light mode=" << mode
              << " id=" << render_id
              << " index=" << index
              << " position=[" << position.GetX() << ' ' << position.GetY() << ' ' << position.GetZ() << ']'
              << " color=[" << intensity.GetRed() << ' ' << intensity.GetGreen() << ' ' << intensity.GetBlue()
              << "]\n";
  }
  for (std::size_t index = 0; index < compiled_scene.world.Objects().size(); ++index) {
    const auto& shape = *compiled_scene.world.Objects()[index];
    const auto& material = shape.GetMaterial();
    const auto& color = material.GetColor();
    const auto world_origin = shape.GetTransform() * Point(0, 0, 0);
    std::cerr << std::fixed << std::setprecision(3)
              << "[scene_editor] runtime object mode=" << mode
              << " id=" << render_id
              << " index=" << index
              << " shape_id=" << shape.GetId()
              << " type=" << RuntimeShapeTypeName(shape)
              << " cast_shadow=" << (shape.GetCastShadow() ? 1 : 0)
              << " origin=[" << world_origin.GetX() << ' ' << world_origin.GetY() << ' ' << world_origin.GetZ() << ']'
              << " color=[" << color.GetRed() << ' ' << color.GetGreen() << ' ' << color.GetBlue() << ']'
              << " ambient=" << material.GetAmbient()
              << " diffuse=" << material.GetDiffuse()
              << " specular=" << material.GetSpecular()
              << " shininess=" << material.GetShininess()
              << " reflective=" << material.GetReflective()
              << " transparency=" << material.GetTransparency()
              << " refractive_index=" << material.GetRefractiveIndex()
              << " pattern=" << (material.HasPattern() ? 1 : 0) << '\n';
  }
}

auto SceneFingerprint(const scene::Scene& scene) -> std::uint64_t {
  std::uint64_t value = 1469598103934665603ULL;
  if (scene.camera.has_value()) {
    value = MixVec3(value, scene.camera->from);
    value = MixVec3(value, scene.camera->to);
    value = MixVec3(value, scene.camera->up);
    value = MixFingerprint(value, Quantize(scene.camera->fov_deg));
    value = MixFingerprint(value, static_cast<std::uint64_t>(scene.camera->resolution.width));
    value = MixFingerprint(value, static_cast<std::uint64_t>(scene.camera->resolution.height));
  }
  value = MixFingerprint(value, scene.lights.size());
  for (const auto& light : scene.lights) {
    value = MixVec3(value, light.position);
    value = MixColor(value, light.color);
  }
  value = MixFingerprint(value, scene.objects.size());
  for (const auto& object : scene.objects) {
    value = MixFingerprint(value, static_cast<std::uint64_t>(object.type));
    value = MixFingerprint(value, object.visible ? 1U : 0U);
    value = MixFingerprint(value, object.cast_shadow ? 1U : 0U);
    value = MixTransform(value, object.transform);
    value = MixColor(value, object.material.color);
    value = MixFingerprint(value, Quantize(object.material.ambient));
    value = MixFingerprint(value, Quantize(object.material.diffuse));
    value = MixFingerprint(value, Quantize(object.material.specular));
    value = MixFingerprint(value, Quantize(object.material.shininess));
    value = MixFingerprint(value, Quantize(object.material.reflective));
    value = MixFingerprint(value, object.material.pattern.has_value() ? 1U : 0U);
  }
  return value;
}

auto LogRayFingerprint(const char* label, int render_id, Camera camera) -> void {
  const int width = camera.GetHorizontalSize();
  const int height = camera.GetVerticalSize();
  const auto top_left = camera.GetRayForPixel(0, 0);
  const auto center = camera.GetRayForPixel(width / 2, height / 2);
  const auto bottom_right = camera.GetRayForPixel(width - 1, height - 1);
  const auto& origin = center.GetOrigin();
  const auto& top_left_dir = top_left.GetDirection();
  const auto& center_dir = center.GetDirection();
  const auto& bottom_right_dir = bottom_right.GetDirection();
  std::cerr << std::fixed << std::setprecision(6)
            << "[scene_editor] camera fingerprint " << label
            << " id=" << render_id
            << " res=" << width << 'x' << height
            << " pixel=" << std::setprecision(9) << camera.GetPixelSize()
            << std::setprecision(6)
            << " origin=[" << origin.GetX() << ' ' << origin.GetY() << ' ' << origin.GetZ() << ']'
            << " tl_dir=[" << top_left_dir.GetX() << ' ' << top_left_dir.GetY() << ' ' << top_left_dir.GetZ() << ']'
            << " center_dir=[" << center_dir.GetX() << ' ' << center_dir.GetY() << ' ' << center_dir.GetZ() << ']'
            << " br_dir=[" << bottom_right_dir.GetX() << ' ' << bottom_right_dir.GetY() << ' '
            << bottom_right_dir.GetZ() << "]\n";
}

}  // namespace ray_tracer::scene_editor
