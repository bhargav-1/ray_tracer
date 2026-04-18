#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <numbers>
#include "app/color.h"
#include "app/material.h"
#include "app/plane.h"
#include "app/point.h"
#include "app/ray_tracing_engine.h"
#include "app/vec3.h"
#include "app/world.h"

namespace ray_tracer {
SCENARIO("11: Reflectivity for default material") {
  GIVEN("the default material") {
    auto material = Material();
    THEN("the reflectivity is 0") {
      REQUIRE(material.GetReflective() == 0.0);
    }
  }
}

SCENARIO("11: Precomputing the reflection vector") {
  GIVEN("shape = plane() r= ray(point(0, 1, -1), vector(0, -sqrt(2)/2, sqrt(2)/2))") {
    auto shape = Plane(1);
    auto ray= Ray(Point(0, 1, -1), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto xs_intersect = shape.Intersect(ray);
    auto comps = PrepareComputations(xs_intersect[0], ray);
    THEN("comps.reflectv = vector(0, sqrt(2)/2, sqrt(2)/2)") {
    REQUIRE(comps.GetReflectV() == Vec3(0, std::sqrt(2) / 2, std::sqrt(2) / 2));
    }
  }
}

SCENARIO("11: The reflected color for a non-reflective material") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, 0), Vec3(0, 1, 0));
    auto shape = Plane(1);
    auto material = Material();
    material.SetAmbient(1);
    shape.SetMaterial(material);
    world.AddObject(std::make_shared<Plane>(shape));

    auto intersection = ray_tracer::Intersection(1, shape);
    auto comps = ray_tracer::PrepareComputations(intersection, ray);
    auto color = GetReflectedColor(world, comps);
    THEN("color = material.color") {
      CHECK(color == Color(0,0,0));
    }
  }
}

SCENARIO("11: The reflected color for a reflective material") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, -3), Vec3(0, -std::numbers::sqrt2/2, std::numbers::sqrt2/2));
    auto shape = std::make_shared<Plane>(1);
    shape->SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    Material material;
    material.SetReflective(0.5);
    shape->SetMaterial(material);
    world.AddObject(shape);
    auto intersection = ray_tracer::Intersection(std::numbers::sqrt2, *shape);
    auto comps = ray_tracer::PrepareComputations(intersection, ray);
    auto color = GetReflectedColor(world, comps);
    THEN("color = Color(0.19032, 0.2379, 0.14274)") {
      CHECK(color == Color(0.19032, 0.2379, 0.14274));
    }
  }
}
} // namespace ray_tracer