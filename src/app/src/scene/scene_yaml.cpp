#include "app/scene/scene_yaml.h"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace ray_tracer::scene_io {
namespace {

auto RequireMap(const YAML::Node& node, const std::string& label) -> void {
  if (!node || !node.IsMap()) {
    throw std::runtime_error(label + " must be a map");
  }
}

auto RequireSequence(const YAML::Node& node, const std::string& label) -> void {
  if (!node || !node.IsSequence()) {
    throw std::runtime_error(label + " must be a sequence");
  }
}

auto Required(const YAML::Node& node, const char* key, const std::string& label) -> YAML::Node {
  const auto child = node[key];
  if (!child) {
    throw std::runtime_error(label + " missing required field '" + std::string(key) + "'");
  }
  return child;
}

auto RequireOnlyKeys(const YAML::Node& node, const std::vector<std::string>& keys, const std::string& label) -> void {
  RequireMap(node, label);
  for (const auto& item : node) {
    const auto key = item.first.as<std::string>();
    if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
      throw std::runtime_error(label + " has unknown field '" + key + "'");
    }
  }
}

auto ParseVec3(const YAML::Node& node, const std::string& label) -> scene::Vec3 {
  RequireSequence(node, label);
  if (node.size() != 3) {
    throw std::runtime_error(label + " must have exactly 3 values");
  }
  return {node[0].as<float>(), node[1].as<float>(), node[2].as<float>()};
}

auto ParseColor(const YAML::Node& node, const std::string& label) -> scene::ColorRgb {
  const auto vec = ParseVec3(node, label);
  return {vec.x, vec.y, vec.z};
}

auto ParseResolution(const YAML::Node& node, const std::string& label) -> scene::Resolution {
  RequireSequence(node, label);
  if (node.size() != 2) {
    throw std::runtime_error(label + " must have exactly 2 values");
  }
  return {node[0].as<int>(), node[1].as<int>()};
}

auto ParseTransform(const YAML::Node& node, const std::string& label) -> scene::Transform {
  RequireMap(node, label);
  scene::Transform transform;
  transform.translate = ParseVec3(Required(node, "translate", label), label + ".translate");
  transform.rotate_deg = ParseVec3(Required(node, "rotate_deg", label), label + ".rotate_deg");
  transform.scale = ParseVec3(Required(node, "scale", label), label + ".scale");
  return transform;
}

auto ObjectTypeFromString(const std::string& type) -> scene::ObjectType {
  if (type == "sphere") {
    return scene::ObjectType::Sphere;
  }
  if (type == "plane") {
    return scene::ObjectType::Plane;
  }
  throw std::runtime_error("unknown object type '" + type + "'");
}

auto PatternTypeFromString(const std::string& type) -> scene::PatternType {
  if (type == "solid") {
    return scene::PatternType::Solid;
  }
  if (type == "stripe") {
    return scene::PatternType::Stripe;
  }
  if (type == "gradient") {
    return scene::PatternType::Gradient;
  }
  if (type == "ring") {
    return scene::PatternType::Ring;
  }
  if (type == "checkers") {
    return scene::PatternType::Checkers;
  }
  if (type == "radial_gradient") {
    return scene::PatternType::RadialGradient;
  }
  if (type == "uv_checkers") {
    return scene::PatternType::UvCheckers;
  }
  if (type == "blended") {
    return scene::PatternType::Blended;
  }
  if (type == "perturbed") {
    return scene::PatternType::Perturbed;
  }
  throw std::runtime_error("unknown pattern type '" + type + "'");
}

auto ObjectTypeToString(scene::ObjectType type) -> const char* {
  switch (type) {
    case scene::ObjectType::Sphere:
      return "sphere";
    case scene::ObjectType::Plane:
      return "plane";
  }
  return "sphere";
}

auto PatternTypeToString(scene::PatternType type) -> const char* {
  switch (type) {
    case scene::PatternType::Solid:
      return "solid";
    case scene::PatternType::Stripe:
      return "stripe";
    case scene::PatternType::Gradient:
      return "gradient";
    case scene::PatternType::Ring:
      return "ring";
    case scene::PatternType::Checkers:
      return "checkers";
    case scene::PatternType::RadialGradient:
      return "radial_gradient";
    case scene::PatternType::UvCheckers:
      return "uv_checkers";
    case scene::PatternType::Blended:
      return "blended";
    case scene::PatternType::Perturbed:
      return "perturbed";
  }
  return "solid";
}

auto ParsePattern(const YAML::Node& node, const std::string& label) -> scene::Pattern {
  RequireMap(node, label);
  scene::Pattern pattern;
  pattern.type = PatternTypeFromString(Required(node, "type", label).as<std::string>());
  if (const auto transform = node["transform"]) {
    pattern.transform = ParseTransform(transform, label + ".transform");
  }
  switch (pattern.type) {
    case scene::PatternType::Solid:
      pattern.color = ParseColor(Required(node, "color", label), label + ".color");
      break;
    case scene::PatternType::Stripe:
    case scene::PatternType::Gradient:
    case scene::PatternType::Ring:
    case scene::PatternType::Checkers:
    case scene::PatternType::Blended: {
      const auto children = Required(node, "children", label);
      RequireSequence(children, label + ".children");
      if (children.size() != 2) {
        throw std::runtime_error(label + ".children must have exactly 2 patterns");
      }
      pattern.children.push_back(ParsePattern(children[0], label + ".children[0]"));
      pattern.children.push_back(ParsePattern(children[1], label + ".children[1]"));
      break;
    }
    case scene::PatternType::UvCheckers: {
      pattern.width = Required(node, "width", label).as<int>();
      pattern.height = Required(node, "height", label).as<int>();
      const auto children = Required(node, "children", label);
      RequireSequence(children, label + ".children");
      if (children.size() != 2) {
        throw std::runtime_error(label + ".children must have exactly 2 patterns");
      }
      pattern.children.push_back(ParsePattern(children[0], label + ".children[0]"));
      pattern.children.push_back(ParsePattern(children[1], label + ".children[1]"));
      break;
    }
    case scene::PatternType::Perturbed:
      pattern.perturb_scale = Required(node, "scale", label).as<float>();
      pattern.children.push_back(ParsePattern(Required(node, "source", label), label + ".source"));
      break;
    case scene::PatternType::RadialGradient: {
      const auto children = Required(node, "children", label);
      RequireSequence(children, label + ".children");
      if (children.size() != 2) {
        throw std::runtime_error(label + ".children must have exactly 2 patterns");
      }
      pattern.children.push_back(ParsePattern(children[0], label + ".children[0]"));
      pattern.children.push_back(ParsePattern(children[1], label + ".children[1]"));
      break;
    }
  }
  return pattern;
}

auto ParseMaterial(const YAML::Node& node, const std::string& label) -> scene::Material {
  RequireMap(node, label);
  scene::Material material;
  material.color = ParseColor(Required(node, "color", label), label + ".color");
  material.ambient = Required(node, "ambient", label).as<float>();
  material.diffuse = Required(node, "diffuse", label).as<float>();
  material.specular = Required(node, "specular", label).as<float>();
  material.shininess = Required(node, "shininess", label).as<float>();
  material.reflective = Required(node, "reflective", label).as<float>();
  material.transparency = Required(node, "transparency", label).as<float>();
  material.refractive_index = Required(node, "refractive_index", label).as<float>();
  if (const auto pattern = node["pattern"]) {
    material.pattern = ParsePattern(pattern, label + ".pattern");
  }
  return material;
}

auto ParseCamera(const YAML::Node& node) -> scene::Camera {
  RequireMap(node, "camera");
  scene::Camera camera;
  camera.from = ParseVec3(Required(node, "from", "camera"), "camera.from");
  camera.to = ParseVec3(Required(node, "to", "camera"), "camera.to");
  camera.up = ParseVec3(Required(node, "up", "camera"), "camera.up");
  camera.fov_deg = Required(node, "fov_deg", "camera").as<float>();
  camera.resolution = ParseResolution(Required(node, "resolution", "camera"), "camera.resolution");
  return camera;
}

auto ParseLight(const YAML::Node& node, const std::string& label) -> scene::Light {
  RequireMap(node, label);
  scene::Light light;
  light.position = ParseVec3(Required(node, "position", label), label + ".position");
  light.color = ParseColor(Required(node, "color", label), label + ".color");
  return light;
}

auto ParseObject(const YAML::Node& node, const std::string& label) -> scene::Object {
  RequireMap(node, label);
  scene::Object object;
  object.name = Required(node, "name", label).as<std::string>();
  object.type = ObjectTypeFromString(Required(node, "type", label).as<std::string>());
  if (const auto visible = node["visible"]) {
    object.visible = visible.as<bool>();
  }
  object.cast_shadow = Required(node, "cast_shadow", label).as<bool>();
  object.transform = ParseTransform(Required(node, "transform", label), label + ".transform");
  object.material = ParseMaterial(Required(node, "material", label), label + ".material");
  return object;
}

auto EmitVec3(YAML::Emitter& out, const scene::Vec3& value) -> void {
  out << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
}

auto EmitColor(YAML::Emitter& out, const scene::ColorRgb& value) -> void {
  out << YAML::Flow << YAML::BeginSeq << value.r << value.g << value.b << YAML::EndSeq;
}

auto EmitTransform(YAML::Emitter& out, const scene::Transform& transform) -> void {
  out << YAML::BeginMap;
  out << YAML::Key << "translate" << YAML::Value;
  EmitVec3(out, transform.translate);
  out << YAML::Key << "rotate_deg" << YAML::Value;
  EmitVec3(out, transform.rotate_deg);
  out << YAML::Key << "scale" << YAML::Value;
  EmitVec3(out, transform.scale);
  out << YAML::EndMap;
}

auto EmitPattern(YAML::Emitter& out, const scene::Pattern& pattern) -> void {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << PatternTypeToString(pattern.type);
  if (pattern.type == scene::PatternType::Solid) {
    out << YAML::Key << "color" << YAML::Value;
    EmitColor(out, pattern.color);
  } else if (pattern.type == scene::PatternType::Perturbed) {
    out << YAML::Key << "scale" << YAML::Value << pattern.perturb_scale;
    out << YAML::Key << "source" << YAML::Value;
    EmitPattern(out, pattern.children.at(0));
  } else {
    if (pattern.type == scene::PatternType::UvCheckers) {
      out << YAML::Key << "width" << YAML::Value << pattern.width;
      out << YAML::Key << "height" << YAML::Value << pattern.height;
    }
    out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;
    for (const auto& child : pattern.children) {
      EmitPattern(out, child);
    }
    out << YAML::EndSeq;
  }
  out << YAML::Key << "transform" << YAML::Value;
  EmitTransform(out, pattern.transform);
  out << YAML::EndMap;
}

auto EmitMaterial(YAML::Emitter& out, const scene::Material& material) -> void {
  out << YAML::BeginMap;
  out << YAML::Key << "color" << YAML::Value;
  EmitColor(out, material.color);
  out << YAML::Key << "ambient" << YAML::Value << material.ambient;
  out << YAML::Key << "diffuse" << YAML::Value << material.diffuse;
  out << YAML::Key << "specular" << YAML::Value << material.specular;
  out << YAML::Key << "shininess" << YAML::Value << material.shininess;
  out << YAML::Key << "reflective" << YAML::Value << material.reflective;
  out << YAML::Key << "transparency" << YAML::Value << material.transparency;
  out << YAML::Key << "refractive_index" << YAML::Value << material.refractive_index;
  if (material.pattern.has_value()) {
    out << YAML::Key << "pattern" << YAML::Value;
    EmitPattern(out, *material.pattern);
  }
  out << YAML::EndMap;
}

}  // namespace

auto ParseSceneYaml(std::string_view yaml_text) -> LoadSceneResult {
  try {
    const auto root = YAML::Load(std::string{yaml_text});
    RequireOnlyKeys(root, {"version", "camera", "lights", "objects"}, "scene");
    scene::Scene scene;
    scene.version = Required(root, "version", "scene").as<int>();
    scene.camera = ParseCamera(Required(root, "camera", "scene"));
    const auto lights = Required(root, "lights", "scene");
    RequireSequence(lights, "lights");
    for (std::size_t index = 0; index < lights.size(); ++index) {
      scene.lights.push_back(ParseLight(lights[index], "lights[" + std::to_string(index) + "]"));
    }
    const auto objects = Required(root, "objects", "scene");
    RequireSequence(objects, "objects");
    for (std::size_t index = 0; index < objects.size(); ++index) {
      scene.objects.push_back(ParseObject(objects[index], "objects[" + std::to_string(index) + "]"));
    }
    return {.ok = true, .scene = std::move(scene), .error = {}};
  } catch (const std::exception& exception) {
    return {.ok = false, .scene = {}, .error = exception.what()};
  }
}

auto LoadSceneYaml(const std::filesystem::path& path) -> LoadSceneResult {
  std::ifstream input{path};
  if (!input) {
    return {.ok = false, .scene = {}, .error = "failed to open scene file '" + path.string() + "'"};
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  auto result = ParseSceneYaml(buffer.str());
  if (!result.ok) {
    result.error = path.string() + ": " + result.error;
  }
  return result;
}

auto SerializeSceneYaml(const scene::Scene& scene) -> std::string {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "version" << YAML::Value << scene.version;
  out << YAML::Key << "camera" << YAML::Value << YAML::BeginMap;
  const auto camera = scene.camera.value_or(scene::Camera{});
  out << YAML::Key << "from" << YAML::Value;
  EmitVec3(out, camera.from);
  out << YAML::Key << "to" << YAML::Value;
  EmitVec3(out, camera.to);
  out << YAML::Key << "up" << YAML::Value;
  EmitVec3(out, camera.up);
  out << YAML::Key << "fov_deg" << YAML::Value << camera.fov_deg;
  out << YAML::Key << "resolution" << YAML::Value << YAML::Flow << YAML::BeginSeq << camera.resolution.width
      << camera.resolution.height << YAML::EndSeq;
  out << YAML::EndMap;
  out << YAML::Key << "lights" << YAML::Value << YAML::BeginSeq;
  for (const auto& light : scene.lights) {
    out << YAML::BeginMap;
    out << YAML::Key << "position" << YAML::Value;
    EmitVec3(out, light.position);
    out << YAML::Key << "color" << YAML::Value;
    EmitColor(out, light.color);
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;
  out << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;
  for (const auto& object : scene.objects) {
    out << YAML::BeginMap;
    out << YAML::Key << "name" << YAML::Value << object.name;
    out << YAML::Key << "type" << YAML::Value << ObjectTypeToString(object.type);
    out << YAML::Key << "visible" << YAML::Value << object.visible;
    out << YAML::Key << "cast_shadow" << YAML::Value << object.cast_shadow;
    out << YAML::Key << "transform" << YAML::Value;
    EmitTransform(out, object.transform);
    out << YAML::Key << "material" << YAML::Value;
    EmitMaterial(out, object.material);
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;
  return std::string{out.c_str()} + "\n";
}

auto SaveSceneYaml(const scene::Scene& scene, const std::filesystem::path& path) -> SaveSceneResult {
  std::ofstream output{path};
  if (!output) {
    return {.ok = false, .error = "failed to open scene file for writing '" + path.string() + "'"};
  }
  output << SerializeSceneYaml(scene);
  if (!output) {
    return {.ok = false, .error = "failed to write scene file '" + path.string() + "'"};
  }
  return {.ok = true, .error = {}};
}

}  // namespace ray_tracer::scene_io
