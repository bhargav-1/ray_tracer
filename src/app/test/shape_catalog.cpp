#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string_view>

#include "app/cone.h"
#include "app/cube.h"
#include "app/scene/scene.h"
#include "app/shape_catalog.h"

namespace ray_tracer {

SCENARIO("shape catalog: describes and creates registered shapes") {
  const auto shapes = shape_catalog::All();
  REQUIRE(shapes.size() == 5);

  const auto& cube = shape_catalog::Find(scene::ObjectType::kCube);
  CHECK(cube.serialized_name == "cube");
  CHECK(shape_catalog::Find(std::string_view{"cube"}).type == scene::ObjectType::kCube);
  CHECK(shape_catalog::DisplayName("smooth_triangle") == "Smooth Triangle");

  const scene::AxialBounds bounds{.minimum = -0.5F, .maximum = 1.5F, .closed = false};
  const auto runtime_cube = cube.create(42, bounds);
  REQUIRE(runtime_cube != nullptr);
  CHECK(runtime_cube->GetId() == 42);
  CHECK(dynamic_cast<const Cube*>(runtime_cube.get()) != nullptr);
  CHECK_FALSE(cube.supports_axial_bounds);

  const auto& cone = shape_catalog::Find(std::string_view{"cone"});
  CHECK(cone.type == scene::ObjectType::kCone);
  CHECK(cone.supports_axial_bounds);
  const auto runtime_cone = cone.create(43, bounds);
  const auto* configured_cone = dynamic_cast<const Cone*>(runtime_cone.get());
  REQUIRE(configured_cone != nullptr);
  CHECK(configured_cone->GetId() == 43);
  CHECK(configured_cone->GetMinimum() == bounds.minimum);
  CHECK(configured_cone->GetMaximum() == bounds.maximum);
  CHECK(configured_cone->IsClosed() == bounds.closed);

  CHECK_THROWS_AS(shape_catalog::Find(std::string_view{"torus"}), std::invalid_argument);
}

}  // namespace ray_tracer
