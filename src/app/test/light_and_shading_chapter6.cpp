#include <app/color.h>
// #include <app/intersection.h>  // for Intersection
#include <app/lighting.h>
#include <app/material.h>
#include <app/matrix.h>                  // for matrix
#include <app/point.h>                   // for tuple
#include <app/point_light.h>             // for point_light
#include <app/sphere.h>                  // for Sphere
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <cmath>
#include <numbers>

namespace ray_tracer {

SCENARIO("Normal on a Sphere at a Point on the X Axis") {
  GIVEN("A Sphere at a Point on the X Axis") {
    THEN("normal at point(1,0,0) = vector(1,0,0)") {
      ray_tracer::Sphere const sphere(1);
      CHECK(sphere.NormalAt(Point(1, 0, 0)) == Vec3(1, 0, 0));
    }
  }
}
SCENARIO("The Normal on a Sphere at a Point on the Y Axis") {
  GIVEN("A Sphere at a Point on the Y Axis") {
    THEN("normal at point(0,1,0) = vector(0,1,0)") {
      Sphere const sphere(1);
      CHECK(sphere.NormalAt(Point(0, 1, 0)) == Vec3(0, 1, 0));
    }
  }
}
SCENARIO("The Normal on a Sphere at a Point on the Z Axis") {
  GIVEN("A Sphere at a Point on the Z Axis") {
    THEN("normal at point(0,0,1) = vector(0,0,1)") {
      Sphere const sphere(1);
      CHECK(sphere.NormalAt(Point(0, 0, 1)) == Vec3(0, 0, 1));
    }
  }
}
SCENARIO("The Normal on a Sphere at a Nonaxial Point") {
  GIVEN("A Sphere at a Nonaxial Point") {
    THEN("normal at point(√3/3,√3/3,√3/3) = vector(√3/3,√3/3,√3/3)") {
      Sphere const sphere(1);
      CHECK(sphere.NormalAt(Point(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3)) ==
            Vec3(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3));
    }
  }
}
SCENARIO("The Normal is a Normalized Vector") {
  GIVEN("A Sphere at a Nonaxial Point") {
    THEN("normal at point(√3/3,√3/3,√3/3) = vector(√3/3,√3/3,√3/3)") {
      Sphere const sphere(1);
      CHECK(sphere.NormalAt(Point(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3)) ==
            Vec3(std::sqrt(3) / 3, std::sqrt(3) / 3, std::sqrt(3) / 3));
    }
  }
}
SCENARIO("Computing the normal on a translated sphere") {
  GIVEN("s = sphere(), set_transform(s, translation(0,1,0))") {
    ray_tracer::Sphere sphere(1);
    sphere.SetTransform(ray_tracer::Matrix<4, 4>::Translation(0, 1, 0));
    THEN("normal at point(0,1.70711,-0.70711) = vector(0,0.70711,-0.70711)") {
      CHECK(sphere.NormalAt(Point(0, 1.70711, -0.70711)) == Vec3(0, 0.70711, -0.70711));
    }
  }
}
SCENARIO("Computing the normal on a transformed sphere") {
  GIVEN("s = sphere(), set_transform(s, scaling(1,0.5,1)*rotation_z(π/5))") {
    ray_tracer::Sphere sphere(1);
    sphere.SetTransform(ray_tracer::Matrix<4, 4>::Scaling(1, 0.5, 1) *
                        ray_tracer::Matrix<4, 4>::RotationZ(std::numbers::pi / 5));
    THEN("normal at point(0,√2/2,-√2/2) = vector(0,0.97014,-0.24254)") {
      CHECK(sphere.NormalAt(Point(0, std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2)) == Vec3(0, 0.97014, -0.24254));
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

SCENARIO("The default material") {
  GIVEN("m = material()") {
    THEN("m.color = color(1,1,1), m.ambient = 0.1, m.diffuse = 0.9, m.specular = 0.9, m.shininess = 200.0") {
      Material const material;
      CHECK(material.GetColor() == Color(1, 1, 1, 0));
      CHECK(material.GetAmbient() == 0.1);
      CHECK(material.GetDiffuse() == 0.9);
      CHECK(material.GetSpecular() == 0.9);
      CHECK(material.GetShininess() == 200.0);
    }
  }
}
SCENARIO("A Sphere has a default material") {
  GIVEN("s = sphere(), m=s.material") {
    THEN("m = material()") {
      Sphere const sphere(1);
      Material const material = sphere.GetMaterial();
      CHECK(material.GetColor() == Color(1, 1, 1, 0));
      CHECK(material.GetAmbient() == 0.1);
      CHECK(material.GetDiffuse() == 0.9);
      CHECK(material.GetSpecular() == 0.9);
      CHECK(material.GetShininess() == 200.0);
    }
  }
}
SCENARIO("A Sphere may be assigned a material") {
  GIVEN("s = sphere(), m = material(), set_material(s, m)") {
    THEN("s.material = m") {
      Sphere sphere(1);
      Material material;
      sphere.SetMaterial(material);
      CHECK(sphere.GetMaterial() == material);
    }
  }
}
SCENARIO("lighting with the eye between the light and the surface") {
  GIVEN("eyev = vector(0,0,-1), normalv = vector(0,0,-1), light = point_light(point(0,0,-10), color(1,1,1))") {
    auto eyev = ray_tracer::Vec3(0, 0, -1);
    auto normalv = ray_tracer::Vec3(0, 0, -1);
    auto light = ray_tracer::PointLight(ray_tracer::Point(0, 0, -10), ray_tracer::Color(1, 1, 1));
    auto material = ray_tracer::Material();
    auto position = ray_tracer::Point(0, 0, 0);
    Sphere const sphere(1);
    auto result = Lighting(material, sphere, light, position, eyev, normalv);
    THEN("result = lighting(m, s, light, position, eyev, normalv)") {
      CHECK(result == ray_tracer::Color(1.9, 1.9, 1.9));
    }
  }
}
SCENARIO("lIghting with the eye between light and surface,eye offset 45 deg") {
  GIVEN("eyev = vector(0,√2/2,-√2/2), normalv = vector(0,0,-1), light = point_light(point(0,0,-10), color(1,1,1))") {
    auto eyev = ray_tracer::Vec3(0, std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2);
    auto normalv = ray_tracer::Vec3(0, 0, -1);
    auto light = ray_tracer::PointLight(ray_tracer::Point(0, 0, -10), ray_tracer::Color(1, 1, 1));
    auto material = ray_tracer::Material();
    auto position = ray_tracer::Point(0, 0, 0);
    Sphere const sphere(1);
    auto result = Lighting(material, sphere, light, position, eyev, normalv);
    THEN("result = lighting(m, s, light, position, eyev, normalv)") {
      CHECK(result == ray_tracer::Color(1.0, 1.0, 1.0));
    }
  }
}
SCENARIO("lighting with eye opposite surface, light offset 45 deg") {
  GIVEN("eyev = vector(0,0,-1), normalv = vector(0,0,-1), light = point_light(point(0,10,-10), color(1,1,1))") {
    auto eyev = ray_tracer::Vec3(0, 0, -1);
    auto normalv = ray_tracer::Vec3(0, 0, -1);
    auto light = ray_tracer::PointLight(ray_tracer::Point(0, 10, -10), ray_tracer::Color(1, 1, 1));
    auto material = ray_tracer::Material();
    auto position = ray_tracer::Point(0, 0, 0);
    Sphere const sphere(1);
    auto result = Lighting(material, sphere, light, position, eyev, normalv);
    THEN("result = lighting(m, s, light, position, eyev, normalv)") {
      CHECK(result == ray_tracer::Color(0.7364, 0.7364, 0.7364));
    }
  }
}
SCENARIO("lighting with eye in the path of the reflection vector") {
  GIVEN(
      "eyev = vector(0, -√2/2, -√2/2), normalv = vector(0,0,-1), light = point_light(point(0,10,-10), color(1,1,1))") {
    auto eyev = ray_tracer::Vec3(0, -std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2);
    auto normalv = ray_tracer::Vec3(0, 0, -1);
    auto light = ray_tracer::PointLight(ray_tracer::Point(0, 10, -10), ray_tracer::Color(1, 1, 1));
    auto material = ray_tracer::Material();
    auto position = ray_tracer::Point(0, 0, 0);
    Sphere const sphere(1);
    auto result = Lighting(material, sphere, light, position, eyev, normalv);
    THEN("result = lighting(m, s, light, position, eyev, normalv)") {
      CHECK(result == ray_tracer::Color(1.6364, 1.6364, 1.6364));
    }
  }
}
SCENARIO("lighting with light behind the surface") {
  GIVEN("eyev = vector(0,0,-1), normalv = vector(0,0,-1), light = point_light(point(0,0,10), color(1,1,1))") {
    auto eyev = ray_tracer::Vec3(0, 0, -1);
    auto normalv = ray_tracer::Vec3(0, 0, -1);
    auto light = ray_tracer::PointLight(ray_tracer::Point(0, 0, 10), ray_tracer::Color(1, 1, 1, 1));
    auto material = ray_tracer::Material();
    auto position = ray_tracer::Point(0, 0, 0);
    Sphere const sphere(1);
    auto result = Lighting(material, sphere, light, position, eyev, normalv);
    THEN("result = lighting(m, s, light, position, eyev, normalv)") {
      CHECK(result == ray_tracer::Color(0.1, 0.1, 0.1));
    }
  }
}
}  // namespace ray_tracer
