#include <app/matrix.h>
#include <app/intersection.h>
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {
SCENARIO("Creating a world") {
  GIVEN("w = world()") {
    THEN("w contains no objects and w has no light source") {
      const auto world = ray_tracer::World();
      CHECK(world.Objects().empty());
      CHECK(world.Lights().empty());
    }
  }
}

SCENARIO("The Default world") {
  GIVEN(
      "light =  point_light(light(-10,10,-10), color(1,1,1), s1 = sphere() with "
      "material(color(0.8,1.0,0.6),diffuse(0.7),specular(0.2))) and s2 = sphere() with "
      "transform(scaling(0.5,0.5,0.5))") {
    THEN("w contains s1 and s2") {
      auto light = ray_tracer::PointLight(ray_tracer::Point(-10, 10, -10), ray_tracer::Color(1, 1, 1));
      auto world = ray_tracer::World::DefaultWorld();
      auto sphere1 = ray_tracer::Sphere();
      auto sphere2 = ray_tracer::Sphere();
      sphere2.SetTransform(ray_tracer::Matrix<4,4>::Scaling(0.5, 0.5, 0.5));
      CHECK(world.Lights().size() == 1);
      CHECK(world.Objects().size() == 2);
      CHECK(world.Lights()[0] == light);
      CHECK(world.Objects()[0] == sphere1);
      CHECK(world.Objects()[1] == sphere2);
    }
  }
}
SCENARIO("7: Intersect a world with a ray") {
  GIVEN("w = default_world(), r = ray(point(0,0,-5), vector(0,0,1))") {
    THEN("xs = intersect_world(w, r)") {
      auto world = ray_tracer::World::DefaultWorld();
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1)) ;
      auto xs_intersect =ray_tracer::Intersect(ray, world);
      CHECK(xs_intersect.size() == 4);
      CHECK(xs_intersect[0].GetTime() == 4);
      CHECK(xs_intersect[1].GetTime() == 4.5);
      CHECK(xs_intersect[2].GetTime() == 5.5);
      CHECK(xs_intersect[3].GetTime() == 6);
    }
  }
}
}  // namespace ray_tracer
