#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>
#include "app/materials/color.h"
#include "app/materials/material.h"
#include "app/primitives/matrix.h"
#include "app/materials/pattern.h"
#include "app/shapes/plane.h"
#include "app/primitives/point.h"
#include "app/rendering/ray_tracing_engine.h"
#include "app/shapes/sphere.h"
#include "app/primitives/utilities.h"
#include "app/primitives/vec3.h"
#include "app/rendering/world.h"
namespace ray_tracer {
class TestPattern : public Pattern {
public:
  [[nodiscard]] auto LocalColorAt(const Point& point) const -> Color override {
    return {point.GetX(), point.GetY(), point.GetZ()};
  }
};

SCENARIO("11: Reflectivity for default material") {
  GIVEN("the default material") {
    auto material = Material();
    THEN("the reflectivity is 0") { REQUIRE(material.GetReflective() == 0.0); }
  }
}

SCENARIO("11: Precomputing the reflection vector") {
  GIVEN("shape = plane() r= ray(point(0, 1, -1), vector(0, -sqrt(2)/2, sqrt(2)/2))") {
    auto shape = Plane(1);
    auto ray = Ray(Point(0, 1, -1), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto xs_intersect = shape.Intersect(ray);
    auto comps = PrepareComputations(xs_intersect[0], ray);
    THEN("comps.reflectv = vector(0, sqrt(2)/2, sqrt(2)/2)") {
      REQUIRE(comps.GetReflectV() == Vec3(0, std::sqrt(2) / 2, std::sqrt(2) / 2));
    }
  }
}

SCENARIO("11: The reflected color for a non-reflective material") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, 0), Vec3(0, 1, 0));
    auto shape = Plane(1);
    auto material = Material();
    material.SetAmbient(1);
    shape.SetMaterial(material);
    world.AddObject(std::make_shared<Plane>(shape));

    auto intersection = ray_tracer::Intersection(1, shape);
    auto comps = ray_tracer::PrepareComputations(intersection, ray);
    auto color = GetReflectedColor(world, comps);
    THEN("color = material.color") { CHECK(color == Color(0, 0, 0)); }
  }
}

SCENARIO("11: The reflected color for a reflective material") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, -3), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto shape = std::make_shared<Plane>(1);
    shape->SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    Material material;
    material.SetReflective(0.5);
    shape->SetMaterial(material);
    world.AddObject(shape);
    auto intersection = ray_tracer::Intersection(std::numbers::sqrt2, *shape);
    auto comps = ray_tracer::PrepareComputations(intersection, ray);
    auto color = GetReflectedColor(world, comps);
    THEN("color = Color(0.19032, 0.2379, 0.14274)") { CHECK(color == Color(0.19032, 0.2379, 0.14274)); }
  }
}

SCENARIO("11: shade_hit() with a reflective material") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, -3), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto shape = std::make_shared<Plane>(1);
    shape->SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    Material material;
    material.SetReflective(0.5);
    shape->SetMaterial(material);
    world.AddObject(shape);
    auto intersection = ray_tracer::Intersection(std::numbers::sqrt2, *shape);
    auto comps = ray_tracer::PrepareComputations(intersection, ray);
    auto color = ShadeHit(world, comps);
    THEN("color = Color(0.87677, 0.92436, 0.82918)") { CHECK(color == Color(0.87677, 0.92436, 0.82918)); }
  }
}

SCENARIO("11: color_at() with mutually reflective surfaces") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World();
    world.AddLight(PointLight(Point(0, 0, 0), Color(1, 1, 1)));
    auto ray = Ray(Point(0, 0, 0), Vec3(0, 1, 0));
    auto lower = std::make_shared<Plane>(1);
    lower->SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    Material lower_material;
    lower_material.SetReflective(1);
    lower->SetMaterial(lower_material);
    world.AddObject(lower);
    auto upper = std::make_shared<Plane>(2);
    upper->SetTransform(Matrix<4, 4>::Translation(0, 1, 0));
    Material upper_material;
    upper_material.SetReflective(1);
    upper->SetMaterial(upper_material);
    world.AddObject(upper);
    auto color = ColorAt(world, ray);
    std::cout << "Color for mirrored" << color << "\n";
    THEN("color is not infinite") {
      CHECK(color.GetRed() < std::numeric_limits<double>::infinity());
      CHECK(color.GetGreen() < std::numeric_limits<double>::infinity());
      CHECK(color.GetBlue() < std::numeric_limits<double>::infinity());
    }
  }
}

SCENARIO("The reflected color at the maximum recursvive depth") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto const shape = std::make_shared<Plane>(1);
    shape->SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    Material material;
    material.SetReflective(1);
    shape->SetMaterial(material);
    world.AddObject(shape);
    auto ray = Ray(Point(0, 0, -3), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto comps = ray_tracer::PrepareComputations(ray_tracer::Intersection(std::numbers::sqrt2, *shape), ray);
    auto color = ray_tracer::GetReflectedColor(world, comps, 0);
    THEN("color = Color(0,0,0)") { CHECK(color == Color(0, 0, 0)); }
  }
}

SCENARIO("11: Transparency and Refractive index for the default material") {
  GIVEN("the default material") {
    auto material = Material();
    THEN("transparency = 0 and refractive_index = 1") {
      REQUIRE(material.GetTransparency() == 0.0);
      REQUIRE(material.GetRefractiveIndex() == 1.0);
    }
  }
}

SCENARIO("11: A helper for producing a sphere with a glassy material") {
  GIVEN("glass_sphere") {
    const auto glass_sphere = Sphere::GlassSphere(1);
    THEN("glass_sphere.material.transparency = 1 and glass_sphere.material.refractive_index = 1.5") {
      CHECK(glass_sphere.GetMaterial().GetTransparency() == 1.0);
      CHECK(glass_sphere.GetMaterial().GetRefractiveIndex() == 1.5);
    }
  }
}

SCENARIO("11: Finding n1 and n2 at various intersections") {
  GIVEN("a glass sphere s1, a scaled glass sphere s2, and a translated glass sphere s3") {
    auto sphere1 = Sphere::GlassSphere(1, 1, 1.5);
    sphere1.SetTransform(Matrix<4, 4>::Scaling(2, 2, 2));
    auto sphere2 = Sphere::GlassSphere(2, 1, 2.0);
    sphere2.SetTransform(Matrix<4, 4>::Translation(0, 0, -0.25));
    auto sphere3 = Sphere::GlassSphere(3, 1, 2.5);
    sphere3.SetTransform(Matrix<4, 4>::Translation(0, 0, 0.25));
    std::vector<Intersection> intersections_xs{Intersection(2, sphere1),
                                               Intersection(2.75, sphere2),
                                               Intersection(3.25, sphere3),
                                               Intersection(4.75, sphere2),
                                               Intersection(5.25, sphere3),
                                               Intersection(6, sphere1)};
    THEN("the n1 and n2 values are as follows") {
      std::vector<std::pair<double, double>> expected_n_values{
              {1.0, 1.5}, {1.5, 2.0}, {2.0, 2.5}, {2.5, 2.5}, {2.5, 1.5}, {1.5, 1.0}};
      for (size_t i = 0; i < intersections_xs.size(); ++i) {
        auto comps = PrepareComputations(intersections_xs[i], Ray(Point(0, 0, -4), Vec3(0, 0, 1)), intersections_xs);
        CHECK(comps.GetRefractiveIndexN1() == expected_n_values[i].first);
        CHECK(comps.GetRefractiveIndexN2() == expected_n_values[i].second);
      }
    }
  }
}

SCENARIO("11: The under point is offset below the surface") {
  GIVEN("a ray intersecting a transparent sphere") {
    auto ray = Ray(Point(0, 0, -5), Vec3(0, 0, 1));
    auto shape = Sphere::GlassSphere(1);
    shape.SetTransform(Matrix<4, 4>::Translation(0, 0, 1));
    auto intersection = Intersection(5, shape);
    auto comps = PrepareComputations(intersection, ray);
    THEN("comps.under_point.z > kEpsilon/2 and comps.point.z < comps.under_point.z") {
      CHECK(comps.GetUnderPoint().GetZ() > utilities::kEpsilon / 2);
      CHECK(comps.GetPoint().GetZ() < comps.GetUnderPoint().GetZ());
    }
  }
}

SCENARIO("11: refracted color with an opaque surface") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, -5), Vec3(0, 1, 1));
    auto shape = world.Objects()[0];
    auto intersections_xs = std::vector<Intersection>{Intersection(4, *shape), Intersection(6, *shape)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[0], ray, intersections_xs);
    auto color = ray_tracer::GetRefractedColor(world, comps);
    THEN("color = Color(0,0,0)") { CHECK(color == Color(0, 0, 0)); }
  }
}

SCENARIO("11: refracted color at the maximum recursive depth") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, -5), Vec3(0, 1, 1));
    auto shape = Sphere::GlassSphere(1);
    shape.SetTransform(Matrix<4, 4>::Scaling(2, 2, 2));
    world.AddObject(std::make_shared<Sphere>(shape));
    auto intersections_xs = std::vector<Intersection>{Intersection(4, shape), Intersection(6, shape)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[0], ray, intersections_xs);
    auto color = ray_tracer::GetRefractedColor(world, comps, 0);
    THEN("color = Color(0,0,0)") { CHECK(color == Color(0, 0, 0)); }
  }
}

SCENARIO("11: The refracted color under total internal reflection") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto ray = Ray(Point(0, 0, std::numbers::sqrt2 / 2), Vec3(0, 1, 0));
    auto shape = world.Objects()[0];
    auto material = shape->GetMaterial();
    material.SetTransparency(1);
    material.SetRefractiveIndex(1.5);
    shape->SetMaterial(material);
    auto intersections_xs = std::vector<Intersection>{Intersection(-std::numbers::sqrt2 / 2, *shape),
                                                      Intersection(std::numbers::sqrt2 / 2, *shape)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[1], ray, intersections_xs);
    auto color = ray_tracer::GetRefractedColor(world, comps);
    THEN("color = Color(0,0,0)") { CHECK(color == Color(0, 0, 0)); }
  }
}

SCENARIO("11: The refracted color with a refracted ray") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto shape = world.Objects()[0];
    auto material = shape->GetMaterial();
    material.SetAmbient(1);
    material.SetPattern(std::make_shared<ray_tracer::TestPattern>());
    shape->SetMaterial(material);
    auto shape_2 = world.Objects()[1];
    auto material_2 = shape_2->GetMaterial();
    material_2.SetTransparency(1);
    material_2.SetRefractiveIndex(1.5);
    shape_2->SetMaterial(material_2);
    auto ray = Ray(Point(0, 0, 0.1), Vec3(0, 1, 0));
    auto intersections_xs = std::vector<Intersection>{Intersection(-0.98999, *shape),
                                                      Intersection(-0.48999, *shape_2),
                                                      Intersection(0.48999, *shape_2),
                                                      Intersection(0.98999, *shape)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[2], ray, intersections_xs);
    auto color = ray_tracer::GetRefractedColor(world, comps, 5);
    THEN("color = Color(0,0.99888,0.04725)") { CHECK(color == Color(0, 0.99888, 0.04725)); }
  }
}

SCENARIO("11: shade_hit() with a transparent material") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto world = ray_tracer::World::DefaultWorld();
    auto floor = Plane(1);
    floor.SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    auto material = floor.GetMaterial();
    material.SetTransparency(0.5);
    material.SetRefractiveIndex(1.5);
    floor.SetMaterial(material);
    world.AddObject(std::make_shared<Plane>(floor));

    auto ball = Sphere(2);
    ball.SetTransform(Matrix<4, 4>::Translation(0, -3.5, -0.5));
    auto material_2 = ball.GetMaterial();
    material_2.SetColor({1, 0, 0});
    material_2.SetAmbient(0.5);
    ball.SetMaterial(material_2);
    world.AddObject(std::make_shared<Sphere>(ball));
    auto ray = Ray(Point(0, 0, -3), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto intersections_xs = std::vector<Intersection>{Intersection(std::numbers::sqrt2, floor)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[0], ray, intersections_xs);
    auto color = ray_tracer::ShadeHit(world, comps, 5);
    THEN("color = Color(0.93642,0.68642,0.68642)") { CHECK(color == Color(0.93642, 0.68642, 0.68642)); }
  }
}

SCENARIO("11: The Schlick Approximation under total internal reflection") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto shape = Sphere::GlassSphere(1);
    auto ray = Ray(Point(0, 0, std::numbers::sqrt2 / 2), Vec3(0, 1, 0));
    auto intersections_xs = std::vector<Intersection>{Intersection(-std::numbers::sqrt2 / 2, shape),
                                                      Intersection(std::numbers::sqrt2 / 2, shape)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[1], ray, intersections_xs);
    auto reflectance = ray_tracer::Schlick(comps);
    THEN("reflectance = 1") { CHECK(reflectance == 1.0); }
  }
}

SCENARIO("11: The Schlick Approximation with a perpendicular viewing angle") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto shape = Sphere::GlassSphere(1);
    auto ray = Ray(Point(0, 0, 0), Vec3(0, 1, 0));
    auto intersections_xs = std::vector<Intersection>{Intersection(-1, shape), Intersection(1, shape)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[1], ray, intersections_xs);
    auto reflectance = ray_tracer::Schlick(comps);
    THEN("reflectance = 0.04") { CHECK(utilities::Equal(reflectance, 0.04)); }
  }
}

SCENARIO("11: The Schlick Approximation with a small angle and n2 > n1") {
  GIVEN("w = default world r=ray(point(0,0,0) , vector(0,1,0))") {
    auto shape = Sphere::GlassSphere(1);
    auto ray = Ray(Point(0, 0.99, -2), Vec3(0, 0, 1));
    auto intersections_xs = std::vector<Intersection>{Intersection(1.8589, shape)};
    auto comps = PrepareComputations(intersections_xs[0], ray, intersections_xs);
    auto reflectance = Schlick(comps);
    THEN("reflectance = 0.48873") { CHECK(utilities::Equal(reflectance , 0.48873)); }
  }
}

SCENARIO("11: shade_hit() with a reflective, transparent material") {
  GIVEN("w = default world r = ray 0 0 3 vector 0 -sqrt2/2 , sqrt2/2, And floor reflective , ball") {
    auto world = ray_tracer::World::DefaultWorld();
    auto floor = Plane(1);
    floor.SetTransform(Matrix<4, 4>::Translation(0, -1, 0));
    auto material = floor.GetMaterial();
    material.SetReflective(0.5);
    material.SetTransparency(0.5);
    material.SetRefractiveIndex(1.5);
    floor.SetMaterial(material);
    world.AddObject(std::make_shared<Plane>(floor));
    auto ball = Sphere(2);
    ball.SetTransform(Matrix<4, 4>::Translation(0, -3.5, -0.5));
    auto material_2 = ball.GetMaterial();
    material_2.SetColor({1, 0, 0});
    material_2.SetAmbient(0.5);
    ball.SetMaterial(material_2);
    world.AddObject(std::make_shared<Sphere>(ball));
    auto ray = Ray(Point(0, 0, -3), Vec3(0, -std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2));
    auto intersections_xs = std::vector{Intersection(std::numbers::sqrt2, floor)};
    auto comps = ray_tracer::PrepareComputations(intersections_xs[0], ray, intersections_xs);
    auto color = ray_tracer::ShadeHit(world, comps, 5);
    THEN("color = Color(0.93391, 0.69643, 0.69243)") {
      CHECK(color == Color(0.93391, 0.69643, 0.69243));
    }
  }
}

} // namespace ray_tracer
