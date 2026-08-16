#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string_view>

#include "app/cube.h"
#include "app/scene/scene.h"
#include "app/shape_catalog.h"

namespace ray_tracer {

SCENARIO("shape catalog: describes and creates registered shapes") {
  const auto shapes = shape_catalog::All();
  REQUIRE(shapes.size() == 4);

  const auto& cube = shape_catalog::Find(scene::ObjectType::kCube);
  CHECK(cube.serialized_name == "cube");
  CHECK(shape_catalog::Find(std::string_view{"cube"}).type == scene::ObjectType::kCube);
  CHECK(shape_catalog::DisplayName("smooth_triangle") == "Smooth Triangle");

  const auto runtime_cube = cube.create(42);
  REQUIRE(runtime_cube != nullptr);
  CHECK(runtime_cube->GetId() == 42);
  CHECK(dynamic_cast<const Cube*>(runtime_cube.get()) != nullptr);

  CHECK_THROWS_AS(shape_catalog::Find(std::string_view{"cone"}), std::invalid_argument);
}

}  // namespace ray_tracer
