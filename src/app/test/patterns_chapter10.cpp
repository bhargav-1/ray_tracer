#include <catch2/catch_test_macros.hpp>
#include "app/color.h"
#include "app/pattern.h"
#include "app/point.h"
#include "app/point_light.h"
#include "app/lighting.h"
#include <memory>
#include "app/vec3.h"
#include "app/material.h"

namespace ray_tracer {
SCENARIO("10: Creating a Stripe pattern") {
  GIVEN("pattern = stripe_pattern(white, black)") {
    constexpr ray_tracer::Color kWhite{1, 1, 1};
    constexpr ray_tracer::Color kBlack{0, 0, 0};
    const ray_tracer::StripePattern pattern(kWhite, kBlack);
    THEN("pattern.a = white") { CHECK(pattern.GetA() == kWhite); }
    THEN("pattern.b = black") { CHECK(pattern.GetB() == kBlack); }
  }
}

SCENARIO("10: A stripe pattern is constant in y") {
  GIVEN("pattern = stripe_pattern(white, black)") {
    THEN("stripe_at(pattern, point(0,0,0) = white, same with point(0,1/2,0)") {
      constexpr ray_tracer::Color kWhite{1, 1, 1};
      constexpr ray_tracer::Color kBlack{0, 0, 0};
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
      constexpr ray_tracer::Color kWhite{1, 1, 1};
      constexpr ray_tracer::Color kBlack{0, 0, 0};
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
    constexpr ray_tracer::Color kWhite{1, 1, 1};
    constexpr ray_tracer::Color kBlack{0, 0, 0};
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
    constexpr ray_tracer::Color kWhite{1, 1, 1};
    constexpr ray_tracer::Color kBlack{0, 0, 0};
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
      CHECK(ray_tracer::Lighting(material, kLight, ray_tracer::Point(0.9, 0, 0), kEyev, kNormalv) == kWhite);
      CHECK(ray_tracer::Lighting(material, kLight, ray_tracer::Point(1.1, 0, 0), kEyev, kNormalv) == kBlack);
    }
  }
}

} // namespace ray_tracer