#include <app/materials/color.h>
#include <app/rendering/world.h>
#include <app/rendering/intersection.h>
#include <app/rendering/ray_tracing_engine.h>
#include <app/rendering/lighting.h>
#include <app/shapes/sphere.h>
#include <app/primitives/matrix.h>
#include <app/primitives/utilities.h>
#include <app/rendering/ray.h>
#include <app/materials/material.h>
#include <app/primitives/point.h>                   // for tuple
#include <app/rendering/point_light.h>             // for point_light
#include <app/primitives/tuple.h>                   // for tuple
#include <app/primitives/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <cmath>
#include <memory>


namespace ray_tracer {
SCENARIO("8: Lighting with the surface in shadow") {
  GIVEN("eyev = vector(0,0,-1), normalv = vector(0,0,-1), light = point_light(point(0,0,-10), color(1,1,1)), in_shadow = true") {
    constexpr auto kEyev = ray_tracer::Vec3(0, 0, -1);
    constexpr auto kNormalv = ray_tracer::Vec3(0, 0, -1);
    constexpr auto kLight = ray_tracer::PointLight(ray_tracer::Point(0, 0, -10), ray_tracer::Color(1, 1, 1));
    const auto material = ray_tracer::Material();
    constexpr auto kPosition = ray_tracer::Point(0, 0, 0);
    constexpr bool kInShadow = true;
    Sphere const sphere(1);
    auto result = Lighting(material, sphere, kLight, kPosition, kEyev, kNormalv, kInShadow);
    THEN("result = lighting(m, s, light, position, eyev, normalv, in_shadow)") {
      CHECK(result == ray_tracer::Color(0.1, 0.1, 0.1));
    }
  }
}
SCENARIO("8: No shadow when nothing is collinear with point and light") {
  GIVEN("point(0,10,0), light = point_light(point(0,0,-10), color(1,1,1))") {
    constexpr auto kPoint = ray_tracer::Point(0, 10, 0);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is no shadow") {
      CHECK_FALSE(IsShadowed(world, kPoint, world.Lights()[0]));
    }
  }
}

SCENARIO("8: The shadow when an object is between the point and the light") {
  GIVEN("point(10,-10,10), light = point_light(point(0,0,-10), color(1,1,1))") {
    constexpr auto kPoint = ray_tracer::Point(10, -10, 10);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is a shadow") {
      CHECK(IsShadowed(world, kPoint, world.Lights()[0]));
    }
  }
}

SCENARIO("8: There is no shadow when an object is behind the light") {
  GIVEN("w =defaultWorld, point(-20,20,-20))") {
    constexpr auto kPoint = ray_tracer::Point(-20, 20, -20);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is no shadow") {
      CHECK_FALSE(IsShadowed(world, kPoint, world.Lights()[0]));
    }
  }
}

SCENARIO("8: There  is no shadow when an object is behind the point") {
  GIVEN("w =defaultWorld, point(-2,2,-2))") {
    constexpr auto kPoint = ray_tracer::Point(-2, 2, -2);
    const auto world = ray_tracer::World::DefaultWorld();
    THEN("there is no shadow") {
      CHECK_FALSE(IsShadowed(world, kPoint, world.Lights()[0]));
    }
  }
}

SCENARIO("8: ShadeHit() is given an intersection in shadow") {
  GIVEN("w =World(), w.light() = point_light(point(0,0,-10), color(1,1,1)), s1 = sphere(), s2 = sphere() with translation(0,0,10), w.add_object(s1), w.add_object(s2), r = ray(point(0,0,5), vector(0,0,1)), i = intersection(4,s2), comps = prepare_computations(i,r)") {
    World world;
    world.AddLight(PointLight(Point(0, 0, -10), Color(1, 1, 1)));
    auto sphere1 = Sphere(1);
    auto sphere2 = Sphere(2);
    sphere2.SetTransform(Matrix<4, 4>::Translation(0, 0, 10));
    world.AddObject(std::make_shared<Sphere>(sphere1));
    world.AddObject(std::make_shared<Sphere>(sphere2));
    constexpr Ray kRay(Point(0, 0, 5), Vec3(0, 0, 1));
    Intersection const given_intersection(4, sphere2);
    auto comps = PrepareComputations(given_intersection, kRay);
    auto color = ShadeHit(world, comps);
    THEN("shade_hit(w,comps) = color(0.1,0.1,0.1)") {
      CHECK(color == Color(0.1, 0.1, 0.1));
    }
  }
}

SCENARIO("8: The hit should offset the point") {
  GIVEN("ray = ray(Point(0,0,-5), Vec3(0,0,1)), sphere with translation(0,0,1), intersection at t=5, comps = prepare_computations(intersection, ray)") {
    constexpr Ray kRay(Point(0, 0, -5), Vec3(0, 0, 1));
    auto sphere = Sphere(1);
    sphere.SetTransform(Matrix<4, 4>::Translation(0, 0, 1));
    const Intersection given_intersection(5, sphere);
    auto comps = PrepareComputations(given_intersection, kRay);
    THEN("comps.point.z < -epsilon/2 and comps.over_point.z > epsilon/2") {
      CHECK(comps.GetOverPoint().GetZ() < utilities::kEpsilon/ 2);
      CHECK(comps.GetPoint().GetZ() > comps.GetOverPoint().GetZ());
    }
  }
}

}
