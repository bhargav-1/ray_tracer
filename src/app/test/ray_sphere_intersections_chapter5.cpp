#include <app/matrix.h>                  // for matrix
#include <app/point.h>                   // for tuple
#include <app/ray.h>                     // for Ray
#include <app/sphere.h>                  // for Sphere
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {
SCENARIO("Creating and Querying a Ray") {
  GIVEN("origin = point(1,2,3), direction = vector(4,5,6)") {
    THEN("Ray(origin, direction) = Ray(point(1,2,3), vector(4,5,6))") {
      ray_tracer::Point origin = ray_tracer::Point(1, 2, 3);
      ray_tracer::Vec3 direction = ray_tracer::Vec3(4, 5, 6);
      ray_tracer::Ray ray(origin, direction);
      CHECK(ray.GetOrigin() == origin);
      CHECK(ray.GetDirection() == direction);
    }
  }
}
SCENARIO("Computing a Point from a Distance") {
  GIVEN("r = Ray(point(2,3,4), vector(1,0,0))") {
    ray_tracer::Ray ray(ray_tracer::Point(2, 3, 4), ray_tracer::Vec3(1, 0, 0));
    THEN("position(r, 0) = point(2,3,4)") { CHECK(ray.Position(0) == ray_tracer::Point(2, 3, 4)); }
    THEN("position(r, 1) = point(3,3,4)") { CHECK(ray.Position(1) == ray_tracer::Point(3, 3, 4)); }
    THEN("position(r, -1) = point(1,3,4)") { CHECK(ray.Position(-1) == ray_tracer::Point(1, 3, 4)); }
    THEN("position(r, 2.5) = point(4.5,3,4)") { CHECK(ray.Position(2.5) == ray_tracer::Point(4.5, 3, 4)); }
  }
}
SCENARIO("A Ray intersects a sphere at two points") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = s.Intersect(ray);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = 4.0") { CHECK(xs[0] == 4.0); }
    THEN("xs[1] = 6.0") { CHECK(xs[1] == 6.0); }
  }
}
SCENARIO("A Ray intersects a sphere at a tangent") {
  GIVEN("r = Ray(point(0,1,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 1, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = s.Intersect(ray);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = 5.0") { CHECK(xs[0] == 5.0); }
    THEN("xs[1] = 5.0") { CHECK(xs[1] == 5.0); }
  }
}

SCENARIO("A Ray misses a sphere") {
  GIVEN("r = Ray(point(0,2,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 2, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = s.Intersect(ray);
    THEN("xs.count = 0") { CHECK(xs.empty()); }
  }
}

SCENARIO("A Ray originates inside a sphere") {
  GIVEN("r = Ray(point(0,0,0), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, 0), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = s.Intersect(ray);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = -1.0") { CHECK(xs[0] == -1.0); }
    THEN("xs[1] = 1.0") { CHECK(xs[1] == 1.0); }
  }
}

SCENARIO("A sphere is behind a ray") {
  GIVEN("r = Ray(point(0,0,5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, 5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = s.Intersect(ray);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = -6.0") { CHECK(xs[0] == -6.0); }
    THEN("xs[1] = -4.0") { CHECK(xs[1] == -4.0); }
  }
}
}  // namespace ray_tracer
