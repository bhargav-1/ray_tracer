#include <app/matrix.h>                  // for matrix
#include <app/point.h>                   // for tuple
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {
SCENARIO("Multiply by translation") {
  GIVEN("Transform = translation{5,-3,2}, p = point(-3,4,5)") {
    THEN("Transform * p = point(2,1,7)") {
      auto transform = ray_tracer::Matrix<4, 4>::Translation(5, -3, 2);
      ray_tracer::Point p = ray_tracer::Point(-3, 4, 5);
      CHECK(transform * p == ray_tracer::Point(2, 1, 7));
    }
  }
}
SCENARIO("Multiply by inverse translation") {
  GIVEN("Transform = translation{5,-3,2}, p = point(-3,4,5)") {
    THEN("Transform * p = point(2,1,7)") {
      auto transform = ray_tracer::Matrix<4, 4>::Translation(5, -3, 2);
      ray_tracer::Point p = ray_tracer::Point(-3, 4, 5);
      CHECK(transform * p == ray_tracer::Point(2, 1, 7));
    }
  }
}
SCENARIO("Translation does not affect vectors") {
  GIVEN("Transform = translation{5,-3,2}, v = vector(-3,4,5)") {
    THEN("Transform * v = vector(-3,4,5)") {
      auto transform = ray_tracer::Matrix<4, 4>::Translation(5, -3, 2);
      ray_tracer::Vec3 v(-3, 4, 5);
      CHECK(transform * v == v);
    }
  }
}
}// namespace ray_tracer
