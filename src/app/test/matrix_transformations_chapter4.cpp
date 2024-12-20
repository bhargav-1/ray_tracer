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
SCENARIO("Scaling a point") {
  GIVEN("Transform = scaling{2,3,4}, p = point(-4,6,8)") {
    THEN("Transform * p = point(-8,18,32)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
      ray_tracer::Point p(-4, 6, 8);
      CHECK(transform * p == ray_tracer::Point(-8, 18, 32));
    }
  }
}

SCENARIO("Scaling a vector") {
  GIVEN("Transform = scaling{2,3,4}, v = vector(-4,6,8)") {
    THEN("Transform * v = vector(-8,18,32)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
      ray_tracer::Vec3 v(-4, 6, 8);
      CHECK(transform * v == ray_tracer::Vec3(-8, 18, 32));
    }
  }
}

SCENARIO("Multiply by inverse of scaling matrix") {
  GIVEN("Transform = scaling{2,3,4}, v = vector(-4,6,8)") {
    THEN("Transform * v = vector(-2,2,2)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
      auto inv = transform.Inverse();
      ray_tracer::Vec3 v(-4, 6, 8);
      CHECK(inv * v == ray_tracer::Vec3(-2, 2, 2));
    }
  }
}
SCENARIO("Reflection is scaling by a negative value") {
  GIVEN("Transform = scaling{-1,1,1}, p = point(2,3,4)") {
    THEN("Transform * p = point(-2,3,4)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(-1, 1, 1);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(-2, 3, 4));
    }
 }
}  // namespace ray_tracer
 }
