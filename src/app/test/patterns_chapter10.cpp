#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "app/color.h"
#include "app/lighting.h"
#include "app/material.h"
#include "app/matrix.h"
#include "app/pattern.h"
#include "app/point.h"
#include "app/point_light.h"
#include "app/sphere.h"
#include "app/vec3.h"

namespace ray_tracer {
constexpr ray_tracer::Color kWhite{1, 1, 1};
constexpr ray_tracer::Color kBlack{0, 0, 0};

SCENARIO("10: Creating a Stripe pattern") {
  GIVEN("pattern = stripe_pattern(white, black)") {
    const ray_tracer::StripePattern pattern(kWhite, kBlack);
    THEN("pattern.a = white") { CHECK(pattern.GetA() == kWhite); }
    THEN("pattern.b = black") { CHECK(pattern.GetB() == kBlack); }
  }
}

SCENARIO("10: A stripe pattern is constant in y") {
  GIVEN("pattern = stripe_pattern(white, black)") {
    THEN("stripe_at(pattern, point(0,0,0) = white, same with point(0,1/2,0)") {
      const ray_tracer::StripePattern pattern(kWhite, kBlack);
      THEN("stripe_at(pattern, point(0,0,0) = white") {
        CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
      }
      THEN("stripe_at(pattern, point(0,1/2,0) = white") {
        CHECK(pattern.ColorAt(ray_tracer::Point(0, 1, 0)) == kWhite);
        CHECK(pattern.ColorAt(ray_tracer::Point(0, 2, 0)) == kWhite);
      }
    }
  }
}
SCENARIO("10: A stripe pattern is constant in z") {
  GIVEN("pattern = stripe_pattern(white, black)") {
    THEN("stripe_at(pattern, point(0,0,0) = white, same with point(0,0,1/2)") {
      const ray_tracer::StripePattern pattern(kWhite, kBlack);
      THEN("stripe_at(pattern, point(0,0,0) = white") {
        CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
      }
      THEN("stripe_at(pattern, point(0,0,1/2) = white") {
        CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 1)) == kWhite);
      }
    }
  }
}

SCENARIO("10: A stripe pattern alternates in x") {
  GIVEN("Pattern = stripe_pattern(white, black)") {
    const ray_tracer::StripePattern pattern(kWhite, kBlack);
    THEN("stripe_at(pattern, point(0,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
    }
    THEN("stripe_at(pattern, point(0.9,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0.9, 0, 0)) == kWhite);
    }
    THEN("stripe_at(pattern, point(1,0,0) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(1, 0, 0)) == kBlack);
    }
    THEN("stripe_at(pattern, point(-0.1,0,0) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(-0.1, 0, 0)) == kBlack);
    }
    THEN("stripe_at(pattern, point(-1,0,0) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(-1, 0, 0)) == kBlack);
    }
    THEN("stripe_at(pattern, point(-1.1,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(-1.1, 0, 0)) == kWhite);
    }
  }
}

SCENARIO("10: Lighting with a pattern applied") {
  GIVEN(" m.pattern = stripe_pattern(white, black), m.ambient = 1, m.light = point_light(point(0,0,-10), white), m.object = sphere(), eyev = vector(0,0,-1), normalv = vector(0,0,-1)") {
    const ray_tracer::StripePattern pattern(kWhite, kBlack);
    ray_tracer::Material material;
    material.SetPattern(std::make_shared<StripePattern>(pattern));
    material.SetAmbient(1);
    material.SetDiffuse(0);
    material.SetSpecular(0);
    constexpr ray_tracer::Vec3 kEyev(0, 0, -1);
    constexpr ray_tracer::Vec3 kNormalv(0, 0, -1);
    constexpr ray_tracer::PointLight kLight(ray_tracer::Point(0, 0, -10), kWhite);
    THEN("color_at(m, object, light) = black") {
      ray_tracer::Sphere const sphere(1);
      CHECK(ray_tracer::Lighting(material, sphere, kLight, ray_tracer::Point(0.9, 0, 0), kEyev, kNormalv) == kWhite);
      CHECK(ray_tracer::Lighting(material, sphere, kLight, ray_tracer::Point(1.1, 0, 0), kEyev, kNormalv) == kBlack);
    }
  }
}

SCENARIO("10: Stripes with an object Transformation") {
  GIVEN("object.transform = scaling(2,2,2), pattern = stripe_pattern(white, black)") {
    const ray_tracer::StripePattern pattern(kWhite, kBlack);
    constexpr ray_tracer::Matrix<4, 4> kMat = ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2);
    ray_tracer::Sphere object(1);
    object.SetTransform(kMat);
    THEN("color_at(pattern, object, point(1.5,0,0)) = white") {
      CHECK(ColorAtObject(pattern, object, ray_tracer::Point(1.5, 0, 0)) == kWhite);
    }
  }
}
SCENARIO("10: Stripes with a pattern transformation") {
  GIVEN("object = sphere(), pattern = stripe_pattern(white, black), pattern.transform = scaling(2,2,2)") {
    ray_tracer::StripePattern pattern(kWhite, kBlack);
    constexpr ray_tracer::Matrix<4, 4> kMat = ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2);
    pattern.SetPatternTransform(kMat);
    ray_tracer::Sphere const object(1);
    THEN("color_at(pattern, object, point(1.5,0,0)) = white") {
      CHECK(ColorAtObject(pattern, object, ray_tracer::Point(1.5, 0, 0)) == kWhite);
    }}
}
SCENARIO("10: Stripes with both an object and a pattern transformation") {
  GIVEN("object.transform = scaling(2,2,2), pattern = stripe_pattern(white, black), pattern.transform = translation(0.5,0,0)") {
    ray_tracer::StripePattern pattern(kWhite, kBlack);
    constexpr ray_tracer::Matrix<4, 4> kObjMat = ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2);
    constexpr ray_tracer::Matrix<4, 4> kPatMat = ray_tracer::Matrix<4, 4>::Translation(0.5, 0, 0);
    ray_tracer::Sphere object(1);
    object.SetTransform(kObjMat);
    pattern.SetPatternTransform(kPatMat);
    THEN("color_at(pattern, object, point(2.5,0,0)) = white") {
      CHECK(ColorAtObject(pattern, object, ray_tracer::Point(2.5, 0, 0)) == kWhite);
    }
  }
}

} // namespace ray_tracer