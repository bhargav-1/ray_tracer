#include "app/scene/scene_yaml.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "app/scene/scene.h"
#include "app/shape_catalog.h"

#include <yaml-cpp/emittermanip.h>
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
      auto error = label + " has unknown field '";
      error += key;
      error += "'";
      throw std::runtime_error(error);
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

auto ParseAxialBounds(const YAML::Node& node, const std::string& label) -> scene::AxialBounds {
  RequireOnlyKeys(node, {"minimum", "maximum", "closed"}, label);
  scene::AxialBounds bounds;
  bounds.minimum = Required(node, "minimum", label).as<float>();
  bounds.maximum = Required(node, "maximum", label).as<float>();
  bounds.closed = Required(node, "closed", label).as<bool>();
  if (bounds.minimum >= bounds.maximum) {
    throw std::runtime_error(label + ".minimum must be less than maximum");
  }
  return bounds;
}

auto PatternTypeFromString(const std::string& type) -> scene::PatternType {
  if (type == "solid") {
    return scene::PatternType::kSolid;
  }
  if (type == "stripe") {
    return scene::PatternType::kStripe;
  }
  if (type == "gradient") {
    return scene::PatternType::kGradient;
  }
  if (type == "ring") {
    return scene::PatternType::kRing;
  }
  if (type == "checkers") {
    return scene::PatternType::kCheckers;
  }
  if (type == "radial_gradient") {
    return scene::PatternType::kRadialGradient;
  }
  if (type == "uv_checkers") {
    return scene::PatternType::kUvCheckers;
  }
  if (type == "blended") {
    return scene::PatternType::kBlended;
  }
  if (type == "perturbed") {
    return scene::PatternType::kPerturbed;
  }
  throw std::runtime_error("unknown pattern type '" + type + "'");
}

auto PatternTypeToString(scene::PatternType type) -> const char* {
  switch (type) {
    case scene::PatternType::kSolid:
      return "solid";
    case scene::PatternType::kStripe:
      return "stripe";
    case scene::PatternType::kGradient:
      return "gradient";
    case scene::PatternType::kRing:
      return "ring";
    case scene::PatternType::kCheckers:
      return "checkers";
    case scene::PatternType::kRadialGradient:
      return "radial_gradient";
    case scene::PatternType::kUvCheckers:
      return "uv_checkers";
    case scene::PatternType::kBlended:
      return "blended";
    case scene::PatternType::kPerturbed:
      return "perturbed";
  }
  return "solid";
}

auto ParsePattern(const YAML::Node& node, const std::string& label) -> scene::Pattern {  // NOLINT(misc-no-recursion)
  RequireMap(node, label);
  scene::Pattern pattern;
  pattern.type = PatternTypeFromString(Required(node, "type", label).as<std::string>());
  if (const auto transform = node["transform"]) {
    pattern.transform = ParseTransform(transform, label + ".transform");
  }
  switch (pattern.type) {
    case scene::PatternType::kSolid:
      pattern.color = ParseColor(Required(node, "color", label), label + ".color");
      break;
    case scene::PatternType::kStripe:
    case scene::PatternType::kGradient:
    case scene::PatternType::kRing:
    case scene::PatternType::kCheckers:
    case scene::PatternType::kBlended: {
      const auto children = Required(node, "children", label);
      RequireSequence(children, label + ".children");
      if (children.size() != 2) {
        throw std::runtime_error(label + ".children must have exactly 2 patterns");
      }
      pattern.children.push_back(ParsePattern(children[0], label + ".children[0]"));
      pattern.children.push_back(ParsePattern(children[1], label + ".children[1]"));
      break;
    }
    case scene::PatternType::kUvCheckers: {
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
    case scene::PatternType::kPerturbed:
      pattern.perturb_scale = Required(node, "scale", label).as<float>();
      pattern.children.push_back(ParsePattern(Required(node, "source", label), label + ".source"));
      break;
    case scene::PatternType::kRadialGradient: {
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
  if (const auto visible = node["visible"]) {
    light.visible = visible.as<bool>();
  }
  return light;
}

auto ParseObject(const YAML::Node& node, const std::string& label) -> scene::Object {  // NOLINT(misc-no-recursion)
  RequireMap(node, label);
  scene::Object object;
  object.name = Required(node, "name", label).as<std::string>();
  const auto type_name = Required(node, "type", label).as<std::string>();
  object.transform = ParseTransform(Required(node, "transform", label), label + ".transform");
  if (const auto visible = node["visible"]) {
    object.visible = visible.as<bool>();
  }

  if (type_name == "group") {
    RequireOnlyKeys(node, {"name", "type", "visible", "transform", "children"}, label);
    scene::GroupSpec group;
    const auto children = Required(node, "children", label);
    RequireSequence(children, label + ".children");
    for (std::size_t index = 0; index < children.size(); ++index) {
      group.children.push_back(ParseObject(children[index], label + ".children[" + std::to_string(index) + "]"));
    }
    object.content = std::move(group);
    return object;
  }

  if (type_name == "csg") {
    RequireOnlyKeys(node, {"name", "type", "visible", "transform", "operation", "children", "material", "cast_shadow"}, label);
    scene::CsgSpec csg;
    const auto operation = Required(node, "operation", label).as<std::string>();
    if (operation == "union") {
      csg.operation = scene::CsgOperation::kUnion;
    } else if (operation == "intersection") {
      csg.operation = scene::CsgOperation::kIntersection;
    } else if (operation == "difference") {
      csg.operation = scene::CsgOperation::kDifference;
    } else {
      throw std::runtime_error(label + ".operation must be union, intersection, or difference");
    }
    const auto children = Required(node, "children", label);
    RequireSequence(children, label + ".children");
    if (children.size() != 2) {
      throw std::runtime_error(label + ".children must contain exactly two operands");
    }
    for (std::size_t index = 0; index < children.size(); ++index) {
      csg.children.push_back(ParseObject(children[index], label + ".children[" + std::to_string(index) + "]"));
      if (!csg.children.back().visible) {
        throw std::runtime_error(label + ".children operands must be visible");
      }
    }
    if (const auto material = node["material"]) {
      csg.material = ParseMaterial(material, label + ".material");
    }
    if (const auto cast_shadow = node["cast_shadow"]) {
      csg.cast_shadow = cast_shadow.as<bool>();
    }
    object.content = std::move(csg);
    return object;
  }

  RequireOnlyKeys(
      node, {"name", "type", "bounds", "vertices", "normals", "source", "visible", "cast_shadow", "transform", "material"}, label);
  const auto& registration = shape_catalog::Find(type_name);
  scene::ShapeSpec shape;
  shape.type = registration.type;
  if (shape.type == scene::ObjectType::kObj) {
    shape.source = Required(node, "source", label).as<std::string>();
    if (shape.source.empty()) {
      throw std::runtime_error(label + ".source must not be empty");
    }
  } else if (node["source"]) {
    throw std::runtime_error(label + ".source is only supported for OBJ objects");
  }
  if (const auto vertices = node["vertices"]) {
    if (shape.type != scene::ObjectType::kTriangle && shape.type != scene::ObjectType::kSmoothTriangle) {
      throw std::runtime_error(label + ".vertices is only supported for triangles");
    }
    RequireSequence(vertices, label + ".vertices");
    if (vertices.size() != 3) {
      throw std::runtime_error(label + ".vertices must contain exactly 3 points");
    }
    for (std::size_t index = 0; index < shape.vertices.size(); ++index) {
      shape.vertices[index] = ParseVec3(vertices[index], label + ".vertices[" + std::to_string(index) + "]");
    }
  }
  if (const auto normals = node["normals"]) {
    if (shape.type != scene::ObjectType::kSmoothTriangle) {
      throw std::runtime_error(label + ".normals is only supported for smooth triangles");
    }
    RequireSequence(normals, label + ".normals");
    if (normals.size() != 3) {
      throw std::runtime_error(label + ".normals must contain exactly 3 vectors");
    }
    for (std::size_t index = 0; index < shape.normals.size(); ++index) {
      shape.normals[index] = ParseVec3(normals[index], label + ".normals[" + std::to_string(index) + "]");
    }
  }
  if (const auto bounds = node["bounds"]) {
    if (!registration.supports_axial_bounds) {
      throw std::runtime_error(label + ".bounds is not supported for type '" +
                               std::string{registration.serialized_name} + "'");
    }
    shape.axial_bounds = ParseAxialBounds(bounds, label + ".bounds");
  }
  shape.cast_shadow = Required(node, "cast_shadow", label).as<bool>();
  shape.material = ParseMaterial(Required(node, "material", label), label + ".material");
  object.content = std::move(shape);
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

auto EmitAxialBounds(YAML::Emitter& out, const scene::AxialBounds& bounds) -> void {
  out << YAML::BeginMap;
  out << YAML::Key << "minimum" << YAML::Value << bounds.minimum;
  out << YAML::Key << "maximum" << YAML::Value << bounds.maximum;
  out << YAML::Key << "closed" << YAML::Value << bounds.closed;
  out << YAML::EndMap;
}

auto EmitPattern(YAML::Emitter& out, const scene::Pattern& pattern) -> void {  // NOLINT(misc-no-recursion)
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << PatternTypeToString(pattern.type);
  if (pattern.type == scene::PatternType::kSolid) {
    out << YAML::Key << "color" << YAML::Value;
    EmitColor(out, pattern.color);
  } else if (pattern.type == scene::PatternType::kPerturbed) {
    out << YAML::Key << "scale" << YAML::Value << pattern.perturb_scale;
    out << YAML::Key << "source" << YAML::Value;
    EmitPattern(out, pattern.children.at(0));
  } else {
    if (pattern.type == scene::PatternType::kUvCheckers) {
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

auto EmitObject(YAML::Emitter& out, const scene::Object& object) -> void {  // NOLINT(misc-no-recursion)
  out << YAML::BeginMap;
  out << YAML::Key << "name" << YAML::Value << object.name;
  out << YAML::Key << "type" << YAML::Value;
  if (object.IsGroup()) {
    out << "group";
  } else if (object.IsCsg()) {
    out << "csg";
  } else {
    out << shape_catalog::Find(object.Shape().type).serialized_name;
  }
  out << YAML::Key << "visible" << YAML::Value << object.visible;
  out << YAML::Key << "transform" << YAML::Value;
  EmitTransform(out, object.transform);
  if (object.IsContainer()) {
    if (object.IsCsg()) {
      const auto& csg = object.Csg();
      out << YAML::Key << "operation" << YAML::Value;
      switch (csg.operation) {
        case scene::CsgOperation::kUnion: out << "union"; break;
        case scene::CsgOperation::kIntersection: out << "intersection"; break;
        case scene::CsgOperation::kDifference: out << "difference"; break;
      }
      out << YAML::Key << "cast_shadow" << YAML::Value << csg.cast_shadow;
      if (csg.material.has_value()) {
        out << YAML::Key << "material" << YAML::Value;
        EmitMaterial(out, *csg.material);
      }
    }
    out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;
    for (const auto& child : object.Children()) {
      EmitObject(out, child);
    }
    out << YAML::EndSeq;
  } else {
    const auto& shape = object.Shape();
    const auto& registration = shape_catalog::Find(shape.type);
    if (shape.type == scene::ObjectType::kObj) {
      out << YAML::Key << "source" << YAML::Value << shape.source;
    }
    if (shape.type == scene::ObjectType::kTriangle || shape.type == scene::ObjectType::kSmoothTriangle) {
      out << YAML::Key << "vertices" << YAML::Value << YAML::BeginSeq;
      for (const auto& vertex : shape.vertices) {
        EmitVec3(out, vertex);
      }
      out << YAML::EndSeq;
    }
    if (shape.type == scene::ObjectType::kSmoothTriangle) {
      out << YAML::Key << "normals" << YAML::Value << YAML::BeginSeq;
      for (const auto& normal : shape.normals) {
        EmitVec3(out, normal);
      }
      out << YAML::EndSeq;
    }
    if (registration.supports_axial_bounds) {
      out << YAML::Key << "bounds" << YAML::Value;
      EmitAxialBounds(out, shape.axial_bounds);
    }
    out << YAML::Key << "cast_shadow" << YAML::Value << shape.cast_shadow;
    out << YAML::Key << "material" << YAML::Value;
    EmitMaterial(out, shape.material);
  }
  out << YAML::EndMap;
}

// NOLINTNEXTLINE(misc-no-recursion)
auto ResolveObjSources(std::vector<scene::Object>& objects, const std::filesystem::path& directory)
    -> void {
  for (auto& object : objects) {
    if (object.IsContainer()) {
      ResolveObjSources(object.Children(), directory);
    } else if (object.Shape().type == scene::ObjectType::kObj) {
      object.Shape().source = (directory / object.Shape().source).lexically_normal().string();
    }
  }
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
  const std::ifstream input{path};
  if (!input) {
    return {.ok = false, .scene = {}, .error = "failed to open scene file '" + path.string() + "'"};
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  auto result = ParseSceneYaml(buffer.str());
  if (!result.ok) {
    result.error = path.string() + ": " + result.error;
  } else {
    // Keep references valid even if the editor later saves the scene elsewhere.
    ResolveObjSources(result.scene.objects, std::filesystem::absolute(path).parent_path());
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
    out << YAML::Key << "visible" << YAML::Value << light.visible;
    out << YAML::Key << "position" << YAML::Value;
    EmitVec3(out, light.position);
    out << YAML::Key << "color" << YAML::Value;
    EmitColor(out, light.color);
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;
  out << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;
  for (const auto& object : scene.objects) {
    EmitObject(out, object);
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
