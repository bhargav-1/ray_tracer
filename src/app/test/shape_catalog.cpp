#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string_view>

#include "app/cone.h"
#include "app/cube.h"
#include "app/point.h"
#include "app/scene/scene.h"
#include "app/shape_catalog.h"
#include "app/triangle.h"
#include "app/vec3.h"

namespace ray_tracer {

SCENARIO("shape catalog: describes and creates registered shapes") {
  const auto shapes = shape_catalog::All();
  REQUIRE(shapes.size() == 8);

  const auto& cube = shape_catalog::Find(scene::ObjectType::kCube);
  CHECK(cube.serialized_name == "cube");
  CHECK(shape_catalog::Find(std::string_view{"cube"}).type == scene::ObjectType::kCube);
  CHECK(shape_catalog::DisplayName("smooth_triangle") == "Smooth Triangle");

  const scene::AxialBounds bounds{.minimum = -0.5F, .maximum = 1.5F, .closed = false};
  scene::ShapeSpec spec;
  spec.axial_bounds = bounds;
  const auto runtime_cube = cube.create(42, spec);
  REQUIRE(runtime_cube != nullptr);
  CHECK(runtime_cube->GetId() == 42);
  CHECK(dynamic_cast<const Cube*>(runtime_cube.get()) != nullptr);
  CHECK_FALSE(cube.supports_axial_bounds);

  const auto& cone = shape_catalog::Find(std::string_view{"cone"});
  CHECK(cone.type == scene::ObjectType::kCone);
  CHECK(cone.supports_axial_bounds);
  const auto runtime_cone = cone.create(43, spec);
  const auto* configured_cone = dynamic_cast<const Cone*>(runtime_cone.get());
  REQUIRE(configured_cone != nullptr);
  CHECK(configured_cone->GetId() == 43);
  CHECK(configured_cone->GetMinimum() == bounds.minimum);
  CHECK(configured_cone->GetMaximum() == bounds.maximum);
  CHECK(configured_cone->IsClosed() == bounds.closed);

  const auto& triangle = shape_catalog::Find(std::string_view{"triangle"});
  CHECK(triangle.type == scene::ObjectType::kTriangle);
  CHECK_FALSE(triangle.supports_axial_bounds);
  CHECK(shape_catalog::DisplayName(triangle.type) == "Triangle");
  const auto runtime_triangle = triangle.create(44, spec);
  const auto* configured_triangle = dynamic_cast<const Triangle*>(runtime_triangle.get());
  REQUIRE(configured_triangle != nullptr);
  CHECK(configured_triangle->GetId() == 44);
  CHECK(configured_triangle->GetVertices()[0] == Point(0, 1, 0));
  CHECK(configured_triangle->GetVertices()[1] == Point(-1, 0, 0));
  CHECK(configured_triangle->GetVertices()[2] == Point(1, 0, 0));
  CHECK(configured_triangle->BoundsOf().GetMin() == Point(-1, 0, 0));
  CHECK(configured_triangle->BoundsOf().GetMax() == Point(1, 1, 0));

  const auto& smooth = shape_catalog::Find(std::string_view{"smooth_triangle"});
  CHECK(smooth.type == scene::ObjectType::kSmoothTriangle);
  CHECK_FALSE(smooth.supports_axial_bounds);
  spec.normals = {scene::Vec3{0, 1, 0}, scene::Vec3{-1, 0, 0}, scene::Vec3{1, 0, 0}};
  const auto runtime_smooth = smooth.create(45, spec);
  const auto* configured_smooth = dynamic_cast<const SmoothTriangle*>(runtime_smooth.get());
  REQUIRE(configured_smooth != nullptr);
  CHECK(configured_smooth->GetId() == 45);
  CHECK(configured_smooth->GetVertices() == configured_triangle->GetVertices());
  CHECK(configured_smooth->GetNormals()[0] == Vec3(0, 1, 0));
  CHECK(configured_smooth->GetNormals()[1] == Vec3(-1, 0, 0));
  CHECK(configured_smooth->GetNormals()[2] == Vec3(1, 0, 0));
  spec.normals[0] = {0, 0, 0};
  CHECK_THROWS_AS(smooth.create(46, spec), std::invalid_argument);

  CHECK_THROWS_AS(shape_catalog::Find(std::string_view{"torus"}), std::invalid_argument);
}

}  // namespace ray_tracer
