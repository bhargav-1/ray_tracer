#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "app/scene/scene.h"

namespace ray_tracer {

class Shape;

namespace shape_catalog {

using ShapeCreator = auto (*)(std::size_t, const scene::ShapeSpec&) -> std::shared_ptr<Shape>;

struct ShapeRegistration {
  scene::ObjectType type;
  std::string_view serialized_name;
  bool supports_axial_bounds;
  ShapeCreator create;
};

[[nodiscard]] auto All() -> std::span<const ShapeRegistration>;
[[nodiscard]] auto Find(scene::ObjectType type) -> const ShapeRegistration&;
[[nodiscard]] auto Find(std::string_view serialized_name) -> const ShapeRegistration&;
[[nodiscard]] auto DisplayName(std::string_view serialized_name) -> std::string;
[[nodiscard]] auto DisplayName(scene::ObjectType type) -> std::string;

}  // namespace shape_catalog
}  // namespace ray_tracer
