#include <app/color.h>
#include <app/intersection.h>            // for Intersection
#include <app/matrix.h>                  // for matrix
#include <app/point.h>                   // for tuple
#include <app/point_light.h>             // for point_light
#include <app/ray.h>                     // for Ray
#include <app/sphere.h>                  // for Sphere
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {

SCENARIO("Normal on a Sphere at a Point on the X Axis") {
  GIVEN("A Sphere at a Point on the X Axis") {
    THEN("normal at point(1,0,0) = vector(1,0,0)") {
      ray_tracer::Sphere const sphere;
      CHECK(sphere.NormalAt(Point(1, 0, 0)) == Vec3(1, 0, 0));
    }
  }
}
SCENARIO("The Normal on a Sphere at a Point on the Y Axis") {
  GIVEN("A Sphere at a Point on the Y Axis") {
    THEN("normal at point(0,1,0) = vector(0,1,0)") {
      Sphere const sphere;
      CHECK(sphere.NormalAt(Point(0, 1, 0)) == Vec3(0, 1, 0));
    }
  }
}
SCENARIO("The Normal on a Sphere at a Point on the Z Axis") {
  GIVEN("A Sphere at a Point on the Z Axis") {
    THEN("normal at point(0,0,1) = vector(0,0,1)") {
      Sphere const sphere;
      CHECK(sphere.NormalAt(Point(0, 0, 1)) == Vec3(0, 0, 1));
    }
  }
}
SCENARIO("The Normal on a Sphere at a Nonaxial Point") {
  GIVEN("A Sphere at a Nonaxial Point") {
    THEN("normal at point(√3/3,√3/3,√3/3) = vector(√3/3,√3/3,√3/3)") {
      Sphere const sphere;
      CHECK(sphere.NormalAt(Point(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3)) ==
            Vec3(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3));
    }
  }
}
SCENARIO("The Normal is a Normalized Vector") {
  GIVEN("A Sphere at a Nonaxial Point") {
    THEN("normal at point(√3/3,√3/3,√3/3) = vector(√3/3,√3/3,√3/3)") {
      Sphere const sphere;
      CHECK(sphere.NormalAt(Point(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3)) ==
            Vec3(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3));
    }
  }
}
SCENARIO("Computing the normal on a translated sphere") {
  GIVEN("s = sphere(), set_transform(s, translation(0,1,0))") {
    ray_tracer::Sphere sphere;
    sphere.SetTransform(ray_tracer::Matrix<4, 4>::Translation(0, 1, 0));
    THEN("normal at point(0,1.70711,-0.70711) = vector(0,0.70711,-0.70711)") {
      CHECK(sphere.NormalAt(Point(0, 1.70711, -0.70711)) == Vec3(0, 0.70711, -0.70711));
    }
  }
}
SCENARIO("Computing the normal on a transformed sphere") {
  GIVEN("s = sphere(), set_transform(s, scaling(1,0.5,1)*rotation_z(π/5))") {
    ray_tracer::Sphere sphere;
    sphere.SetTransform(ray_tracer::Matrix<4, 4>::Scaling(1, 0.5, 1) * ray_tracer::Matrix<4, 4>::RotationZ(M_PI / 5));
    THEN("normal at point(0,√2/2,-√2/2) = vector(0,0.97014,-0.24254)") {
      CHECK(sphere.NormalAt(Point(0, std::sqrt(2) / 2, -std::sqrt(2) / 2)) == Vec3(0, 0.97014, -0.24254));
    }
  }
}

SCENARIO("A point light has position and intensity") {
  GIVEN("intensity = color(1,1,1), position = point(0,0,0)") {
    THEN("point_light = point_light(position, intensity)") {
      ray_tracer::Color intensity(1, 1, 1);
      ray_tracer::Point position(0, 0, 0);
      PointLight const point_light(position, intensity);
      CHECK(point_light.GetPosition() == position);
      CHECK(point_light.GetIntensity() == intensity);
    }
  }
}
}  // namespace ray_tracer
