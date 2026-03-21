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

class TestPattern : public Pattern {
public:
  [[nodiscard]] auto ColorAt(const Point& point) const -> Color override {
    return {point.GetX(), point.GetY(), point.GetZ()};
  }
};

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

SCENARIO("10: The default pattern transformation") {
  GIVEN("pattern = test_pattern()") {
    THEN("pattern.transform = identity_matrix") {
      const ray_tracer::TestPattern pattern;
      CHECK(pattern.GetPatternTransform() == ray_tracer::Matrix<4, 4>::IdentityMatrix());
    }
  }
}

SCENARIO("10: Assigning a transformation") {
  GIVEN("pattern = test_pattern(), set_pattern_transform(pattern, translation(1,2,3))") {
    ray_tracer::TestPattern pattern;
    pattern.SetPatternTransform(ray_tracer::Matrix<4, 4>::Translation(1, 2, 3));
    THEN("pattern.transform = translation(1,2,3)") {
      CHECK(pattern.GetPatternTransform() == ray_tracer::Matrix<4, 4>::Translation(1, 2, 3));
    }
  }
}

SCENARIO("10: A pattern with an object transformation") {
  GIVEN("object.transform = scaling(2,2,2), pattern = test_pattern()") {
    ray_tracer::Sphere object(1);
    object.SetTransform(ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2));
    THEN("pattern_at_shape(pattern, object, point(2,3,4)) = color(1,1.5,2)") {
      const ray_tracer::TestPattern pattern;
      CHECK(ColorAtShape(pattern, object, ray_tracer::Point(2, 3, 4)) == ray_tracer::Color(1, 1.5, 2));
    }
  }
}

SCENARIO("10: A pattern with a pattern transformation") {
  GIVEN("pattern = test_pattern(), pattern.transform = scaling(2,2,2)") {
    ray_tracer::TestPattern pattern;
    pattern.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2));
    ray_tracer::Sphere const object(1);
    THEN("pattern_at_shape(pattern, object, point(2,3,4)) = color(1,1.5,2)") {
      CHECK(ColorAtShape(pattern, object, ray_tracer::Point(2, 3, 4)) == ray_tracer::Color(1, 1.5, 2));
    }
  }
}

SCENARIO("10: A pattern with both an object and a pattern transformation") {
  GIVEN("object.transform = scaling(2,2,2), pattern.transform = translation(0.5,1,1.5)") {
    ray_tracer::Sphere object(1);
    object.SetTransform(ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2));
    ray_tracer::TestPattern pattern;
    pattern.SetPatternTransform(ray_tracer::Matrix<4, 4>::Translation(0.5, 1, 1.5));
    THEN("pattern_at_shape(pattern, object, point(2.5,3,3.5)) = color(0.75,0.5,0.25)") {
      CHECK(ColorAtShape(pattern, object, ray_tracer::Point(2.5, 3, 3.5)) == ray_tracer::Color(0.75, 0.5, 0.25));
    }
  }
}

SCENARIO("10: A gradient linearly interpolates between colors") {
  GIVEN("pattern = gradientPattern(white, black)"){
    const ray_tracer::GradientPattern pattern(kWhite, kBlack);
    THEN("pattern_at(pattern, point(0,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(0.25,0,0) = color(0.75,0.75,0.75)") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0.25, 0, 0)) == ray_tracer::Color(0.75, 0.75, 0.75));
    }
    THEN("pattern_at(pattern, point(0.5,0,0) = color(0.5,0.5,0.5)") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0.5, 0, 0)) == ray_tracer::Color(0.5, 0.5, 0.5));
    }
    THEN("pattern_at(pattern, point(0.75,0,0) = color(0.25,0.25,0.25)") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0.75, 0, 0)) == ray_tracer::Color(0.25, 0.25, 0.25));
    }
  }
}

SCENARIO("10: A ring should extend in both x and z") {
  GIVEN("pattern = ring_pattern(white, black)") {
    const ray_tracer::RingPattern pattern(kWhite, kBlack);
    THEN("pattern_at(pattern, point(0,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(1,0,0) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(1, 0, 0)) == kBlack);
    }
    THEN("pattern_at(pattern, point(0,0,1) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 1)) == kBlack);
    }
    THEN("pattern_at(pattern, point(0.708,0,0.708) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0.708, 0, 0.708)) == kBlack);
    }
  }
}

SCENARIO("10: Checkers should repeat in x") {
  GIVEN("pattern = checkers_pattern(white, black)") {
    const ray_tracer::CheckersPattern pattern(kWhite, kBlack);
    THEN("pattern_at(pattern, point(0,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(0.99,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0.99, 0, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(1.01,0,0) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(1.01, 0, 0)) == kBlack);
    }
  }
}

SCENARIO("10: Checkers should repear in y") {
  GIVEN("pattern = checkers_pattern(white, black)") {
    const ray_tracer::CheckersPattern pattern(kWhite, kBlack);
    THEN("pattern_at(pattern, point(0,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(0,0.99,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0.99, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(0,1.01,0) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 1.01, 0)) == kBlack);
    }
  }
}

SCENARIO("10: Checkers should repeat in z") {
  GIVEN("pattern = checkers_pattern(white, black)") {
    const ray_tracer::CheckersPattern pattern(kWhite, kBlack);
    THEN("pattern_at(pattern, point(0,0,0) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0)) == kWhite);
    }
    THEN("pattern_at(pattern, point(0,0,0.99) = white") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 0.99)) == kWhite);
    }
    THEN("pattern_at(pattern, point(0,0,1.01) = black") {
      CHECK(pattern.ColorAt(ray_tracer::Point(0, 0, 1.01)) == kBlack);
    }
  }
}

} // namespace ray_tracer