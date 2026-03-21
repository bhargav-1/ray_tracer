#include <catch2/catch_test_macros.hpp>
#include "app/point.h"
#include "app/color.h"
#include "app/pattern.h"

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
        CHECK(pattern.StripeAt(ray_tracer::Point(0, 0, 0)) == kWhite);
      }
      THEN("stripe_at(pattern, point(0,1/2,0) = white") {
        CHECK(pattern.StripeAt(ray_tracer::Point(0, 1, 0)) == kWhite);
        CHECK(pattern.StripeAt(ray_tracer::Point(0, 2, 0)) == kWhite);
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
        CHECK(pattern.StripeAt(ray_tracer::Point(0, 0, 0)) == kWhite);
      }
      THEN("stripe_at(pattern, point(0,0,1/2) = white") {
        CHECK(pattern.StripeAt(ray_tracer::Point(0, 0, 1)) == kWhite);
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
        CHECK(pattern.StripeAt(ray_tracer::Point(0, 0, 0)) == kWhite);
      }
      THEN("stripe_at(pattern, point(0.9,0,0) = white") {
        CHECK(pattern.StripeAt(ray_tracer::Point(0.9, 0, 0)) == kWhite);
      }
      THEN("stripe_at(pattern, point(1,0,0) = black") {
        CHECK(pattern.StripeAt(ray_tracer::Point(1, 0, 0)) == kBlack);
      }
      THEN("stripe_at(pattern, point(-0.1,0,0) = black") {
        CHECK(pattern.StripeAt(ray_tracer::Point(-0.1, 0, 0)) == kBlack);
      }
      THEN("stripe_at(pattern, point(-1,0,0) = black") {
        CHECK(pattern.StripeAt(ray_tracer::Point(-1, 0, 0)) == kBlack);
      }
      THEN("stripe_at(pattern, point(-1.1,0,0) = white") {
        CHECK(pattern.StripeAt(ray_tracer::Point(-1.1, 0, 0)) == kWhite);
      }
  }
}

}