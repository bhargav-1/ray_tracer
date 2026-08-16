#include <catch2/catch_test_macros.hpp>

// #include "app/cylinder.h"
#include "app/cylinder.h"
#include "app/point.h"
#include "app/ray.h"
#include "app/utilities.h"
#include "app/vec3.h"

namespace ray_tracer {
SCENARIO("13: A ray misses a cylinder") {
  GIVEN("cyl = cylinder(), dir = normalize(<1, 0, 0>) , ray = ray(<1, 0,0>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 1, 0).Normalize();
      auto ray = Ray(Point(1, 0, 0), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<0, 1, 0>) , ray = ray(<0, 0, 0>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 1, 0).Normalize();
      auto ray = Ray(Point(0, 0, 0), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<1, 1, 1>) , ray = ray(<0, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 1, 0).Normalize();
      auto ray = Ray(Point(0, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
}
SCENARIO("13: A ray intersects a cylinder") {
  GIVEN("cyl = cylinder(), dir = normalize(<0, 0, 1>) , ray = ray(<1, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(1, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
      CHECK(intersections_xs[0].GetTime() == 5);
      CHECK(intersections_xs[1].GetTime() == 5);
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<0, 0, 1>) , ray = ray(<0, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
      CHECK(intersections_xs[0].GetTime() == 4);
      CHECK(intersections_xs[1].GetTime() == 6);
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<0.1, 1, 1>) , ray = ray(<0.5, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0.1, 1, 1).Normalize();
      auto ray = Ray(Point(0.5, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
      CHECK(utilities::Equal(6.80798, intersections_xs[0].GetTime()));
      CHECK(utilities::Equal(7.08872, intersections_xs[1].GetTime()));
    }
  }
}

SCENARIO("13: Normal vector on a cylinder") {
  GIVEN("cyl = cylinder(),normal = local_normal_at(cyl, <point>") {
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(1, 0, 0));
      CHECK(normal == Vec3(1, 0, 0));
    }
  }
  GIVEN("cyl = cylinder(),normal = local_normal_at(cyl, <point>") {
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(0, 5, 0));
      CHECK(normal == Vec3(0, 0, -1));
    }
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(0, -2, 1));
      CHECK(normal == Vec3(0, 0, 1));
    }
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(-1, 1, 0));
      CHECK(normal == Vec3(-1, 0, 0));
    }
  }
}
}  // namespace ray_tracer