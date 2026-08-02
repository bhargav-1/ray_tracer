#include "app/shape_catalog.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include "app/cube.h"
#include "app/plane.h"
#include "app/scene/scene.h"
#include "app/shape.h"
#include "app/sphere.h"

namespace ray_tracer::shape_catalog {
namespace {

template <typename ShapeType>
auto CreateShape(std::size_t object_id) -> std::shared_ptr<Shape> {
  return std::make_shared<ShapeType>(object_id);
}

constexpr std::array kShapes{
    ShapeRegistration{scene::ObjectType::kSphere, "sphere", &CreateShape<Sphere>},
    ShapeRegistration{scene::ObjectType::kPlane, "plane", &CreateShape<Plane>},
    ShapeRegistration{scene::ObjectType::kCube, "cube", &CreateShape<Cube>},
};

}  // namespace

auto All() -> std::span<const ShapeRegistration> {
  return kShapes;
}

auto Find(scene::ObjectType type) -> const ShapeRegistration& {
  for (const auto& shape : kShapes) {
    if (shape.type == type) {
      return shape;
    }
  }
  throw std::invalid_argument("unknown object type");
}

auto Find(std::string_view serialized_name) -> const ShapeRegistration& {
  for (const auto& shape : kShapes) {
    if (shape.serialized_name == serialized_name) {
      return shape;
    }
  }
  throw std::invalid_argument("unknown object type '" + std::string{serialized_name} + "'");
}

auto DisplayName(std::string_view serialized_name) -> std::string {
  std::string display_name;
  display_name.reserve(serialized_name.size());
  bool capitalize = true;
  for (const char character : serialized_name) {
    if (character == '_') {
      display_name.push_back(' ');
      capitalize = true;
      continue;
    }
    const auto value = static_cast<unsigned char>(character);
    display_name.push_back(capitalize ? static_cast<char>(std::toupper(value)) : character);
    capitalize = false;
  }
  return display_name;
}

auto DisplayName(scene::ObjectType type) -> std::string {
  return DisplayName(Find(type).serialized_name);
}

}  // namespace ray_tracer::shape_catalog
