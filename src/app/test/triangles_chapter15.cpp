#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include "app/utilities.h"
// #include "app/matrix.h"
#include "app/triangle.h"

namespace ray_tracer {
SCENARIO("15: A triangle bounding box encloses its vertices") {
  const auto triangle = Triangle(1, {Point(-3, 7, 2), Point(6, 2, -4), Point(2, -1, 5)});
  const auto bounds = triangle.BoundsOf();
  CHECK(bounds.GetMin() == Point(-3, -1, -4));
  CHECK(bounds.GetMax() == Point(6, 7, 5));
}

SCENARIO("15: Constructing a triangle") {
  GIVEN("p1 = point(0, 1, 0); p2 = point(-1, 0, 0); p3 = point(1, 0, 0); t = triangle(p1, p2, p3)") {
    auto point_1 = Point(0, 1, 0);
    auto point_2 = Point(-1, 0, 0);
    auto point_3 = Point(1, 0, 0);
    auto triangle_var = Triangle(1, {point_1, point_2, point_3});
    THEN("t.p1 == p1 and t.p2 == p2 and t.p3 == p3") {
      CHECK(triangle_var.GetVertices()[0] == point_1);
      CHECK(triangle_var.GetVertices()[1] == point_2);
      CHECK(triangle_var.GetVertices()[2] == point_3);
      CHECK(triangle_var.GetEdge1() == Vec3(-1, -1, 0));
      CHECK(triangle_var.GetEdge2() == Vec3(1, -1, 0));
      CHECK(triangle_var.GetNormal() == Vec3(0, 0, -1));
    }
  }
}
SCENARIO("15: Finding the normal on a triangle") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    THEN(
        "n1 = t.normal_at(point(0, 0.5, 0)), n2 = t.normal_at(point(-0.50, 0.75, 0)), n3 = t.normal_at(point(0.5, "
        "0.25, 0))") {
      auto normal_1 = triangle_var.NormalAt(Point(0, 0.5, 0));
      auto normal_2 = triangle_var.NormalAt(Point(-0.5, 0.75, 0));
      auto normal_3 = triangle_var.NormalAt(Point(0.5, 0.25, 0));
      CHECK(normal_1 == triangle_var.GetNormal());
      CHECK(normal_2 == triangle_var.GetNormal());
      CHECK(normal_3 == triangle_var.GetNormal());
    }
  }
}

SCENARIO("15: Intersecting a ray parallel to the triangle") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(0, -1, -2), vec(0, 1, 0))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(0, -1, -2), Vec3(0, 1, 0));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}

SCENARIO("15: A ray misses the p1-p3 edge") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(1, 1, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(1, 1, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}
SCENARIO("15: A ray misses the p1-p2 edge") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(-1, 1, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(-1, 1, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}
SCENARIO("15: A ray misses the p2-p3 edge") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(0, -1, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(0, -1, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}
SCENARIO("15: A ray strikes a triangle") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(0, 0.5, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(0, 0.5, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.size() == 1);
      CHECK(intersections[0].GetTime() - 2.0 < utilities::kEpsilon);
    }
  }
}

}  // namespace ray_tracer
