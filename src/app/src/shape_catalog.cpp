#include "app/shape_catalog.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <cmath>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include "app/cone.h"
#include "app/cube.h"
#include "app/cylinder.h"
#include "app/plane.h"
#include "app/obj_parser.h"
#include "app/point.h"
#include "app/scene/scene.h"
#include "app/shape.h"
#include "app/sphere.h"
#include "app/triangle.h"
#include "app/tuple.h"

namespace ray_tracer::shape_catalog {
namespace {

template <typename ShapeType>
auto CreateShape(std::size_t object_id, [[maybe_unused]] const scene::ShapeSpec& spec) -> std::shared_ptr<Shape> {
  return std::make_shared<ShapeType>(object_id);
}

template <typename ShapeType>
auto CreateAxialShape(std::size_t object_id, const scene::ShapeSpec& spec) -> std::shared_ptr<Shape> {
  const auto& bounds = spec.axial_bounds;
  auto shape = std::make_shared<ShapeType>(object_id);
  shape->SetMinimum(bounds.minimum);
  shape->SetMaximum(bounds.maximum);
  shape->SetClosed(bounds.closed);
  return shape;
}

auto CreateTriangle(std::size_t object_id, const scene::ShapeSpec& spec)
    -> std::shared_ptr<Shape> {
  std::array<Point, 3> vertices;
  for (std::size_t index = 0; index < vertices.size(); ++index) {
    const auto& vertex = spec.vertices[index];
    if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z)) {
      throw std::invalid_argument("triangle vertices must be finite");
    }
    vertices[index] = Point(vertex.x, vertex.y, vertex.z);
  }
  const auto normal = Vec3(vertices[1] - vertices[0]).Cross(Vec3(vertices[2] - vertices[0]));
  if (normal.Dot(normal) == 0) {
    throw std::invalid_argument("triangle vertices must not be collinear or coincident");
  }
  return std::make_shared<Triangle>(object_id, vertices);
}

auto CreateObj(std::size_t object_id, const scene::ShapeSpec& spec) -> std::shared_ptr<Shape> {
  if (spec.source.empty()) {
    throw std::invalid_argument("OBJ object requires a source file");
  }
  const ObjParser parser(spec.source, object_id);
  if (parser.Triangles().empty()) {
    throw std::invalid_argument("OBJ file contains no faces: " + spec.source);
  }
  return parser.ObjToGroup();
}

constexpr std::array kShapes{
    ShapeRegistration{scene::ObjectType::kSphere, "sphere", false, &CreateShape<Sphere>},
    ShapeRegistration{scene::ObjectType::kPlane, "plane", false, &CreateShape<Plane>},
    ShapeRegistration{scene::ObjectType::kCube, "cube", false, &CreateShape<Cube>},
    ShapeRegistration{scene::ObjectType::kCylinder, "cylinder", true, &CreateAxialShape<Cylinder>},
    ShapeRegistration{scene::ObjectType::kCone, "cone", true, &CreateAxialShape<Cone>},
    ShapeRegistration{scene::ObjectType::kTriangle, "triangle", false, &CreateTriangle},
    ShapeRegistration{scene::ObjectType::kObj, "obj", false, &CreateObj},
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
