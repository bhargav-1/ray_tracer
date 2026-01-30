#include <app/color.h>
#include <app/world.h>
#include <app/intersection.h>
#include <app/lighting.h>
#include <app/material.h>
#include <app/point.h>                   // for tuple
#include <app/point_light.h>             // for point_light
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <cmath>


namespace ray_tracer {
SCENARIO("8: Lighting with the surface in shadow") {
  GIVEN("eyev = vector(0,0,-1), normalv = vector(0,0,-1), light = point_light(point(0,0,-10), color(1,1,1)), in_shadow = true") {
    constexpr auto kEyev = ray_tracer::Vec3(0, 0, -1);
    constexpr auto kNormalv = ray_tracer::Vec3(0, 0, -1);
    constexpr auto kLight = ray_tracer::PointLight(ray_tracer::Point(0, 0, -10), ray_tracer::Color(1, 1, 1));
    const auto material = ray_tracer::Material();
    constexpr auto kPosition = ray_tracer::Point(0, 0, 0);
    constexpr bool kInShadow = true;
    auto result = Lighting(material, kLight, kPosition, kEyev, kNormalv, kInShadow);
    THEN("result = lighting(m, s, light, position, eyev, normalv, in_shadow)") {
      CHECK(result == ray_tracer::Color(0.1, 0.1, 0.1));
    }
  }
}
SCENARIO("There is no shadow when nothing is collinear with point and light") {
  GIVEN("point(0,10,0), light = point_light(point(0,0,-10), color(1,1,1))") {
    constexpr auto kPoint = ray_tracer::Point(0, 10, 0);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is no shadow") {
      CHECK_FALSE(IsShadowed(world,kPoint));
    }
  }
}

SCENARIO("The shadow when an object is between the point and the light") {
  GIVEN("point(10,-10,10), light = point_light(point(0,0,-10), color(1,1,1))") {
    constexpr auto kPoint = ray_tracer::Point(10, -10, 10);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is a shadow") {
      CHECK(IsShadowed(world,kPoint));
    }
  }
}

SCENARIO("There is no shadow when an object is between the point and the light") {
  GIVEN("w =defaultWorld, point(-20,20,-20))") {
    constexpr auto kPoint = ray_tracer::Point(-20, 20, -20);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is no shadow") {
      CHECK_FALSE(IsShadowed(world,kPoint));
    }
  }
}

SCENARIO("There  is no shadow when an object is behind the point") {
  GIVEN("w =defaultWorld, point(-2,2,-2))") {
    constexpr auto kPoint = ray_tracer::Point(-2, 2, -2);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is no shadow") {
      CHECK_FALSE(IsShadowed(world,kPoint));
    }
  }
}

}