#include <app/matrix.h>
#include <app/intersection.h>
#include <app/sphere.h>
#include <app/utilities.h>
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {
SCENARIO("7: Creating a world") {
  GIVEN("w = world()") {
    THEN("w contains no objects and w has no light source") {
      const auto world = ray_tracer::World();
      CHECK(world.Objects().empty());
      CHECK(world.Lights().empty());
    }
  }
}

SCENARIO("7: The Default world") {
  GIVEN(
      "light =  point_light(light(-10,10,-10), color(1,1,1), s1 = sphere() with "
      "material(color(0.8,1.0,0.6),diffuse(0.7),specular(0.2))) and s2 = sphere() with "
      "transform(scaling(0.5,0.5,0.5))") {
    THEN("w contains s1 and s2") {
      auto light = ray_tracer::PointLight(ray_tracer::Point(-10, 10, -10), ray_tracer::Color(1, 1, 1));
      auto world = ray_tracer::World::DefaultWorld();
      auto sphere1 = ray_tracer::Sphere(1);
      auto sphere2 = ray_tracer::Sphere(2);
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
SCENARIO("7: Precomputing the state of an intersection") {
  GIVEN("r = ray(point(0,0,-5), vector(0,0,1)), shape = sphere(), i = intersection(4, shape)") {
    THEN("comps = prepare_computations(i, r)") {
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
      auto shape = ray_tracer::Sphere(1);
      auto intersection = ray_tracer::Intersection(4, shape);
      auto comps = ray_tracer::PrepareComputations(intersection, ray);
      CHECK(comps.GetTime() == intersection.GetTime());
      CHECK(comps.GetObject() == intersection.GetObject());
      CHECK(comps.GetPoint() == ray.Position(comps.GetTime()));
      CHECK(comps.GetEyeV() == -ray.GetDirection());
      CHECK(comps.GetNormalV() == ray_tracer::Vec3(0, 0, -1));
    }
  }
}
SCENARIO("7: The hit, when an intersection occurs on the outside") {
  GIVEN("r = ray(point(0,0,-5), vector(0,0,1)), shape = sphere(), i = intersection(4, shape)") {
    THEN("comps = prepare_computations(i, r), comps.inside = false") {
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
      auto shape = ray_tracer::Sphere(1);
      auto intersection = ray_tracer::Intersection(4, shape);
      auto comps = ray_tracer::PrepareComputations(intersection, ray);
      CHECK(comps.IsInside() == false);
    }
  }
}
SCENARIO("7: The hit, when an intersection occurs on the inside") {
  GIVEN("r = ray(point(0,0,0), vector(0,0,1)), shape = sphere(), i = intersection(1, shape)") {
    THEN("comps = prepare_computations(i, r), comps.inside = true, comps.normalv = -normalv") {
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, 0), ray_tracer::Vec3(0, 0, 1));
      auto shape = ray_tracer::Sphere(1);
      auto intersection = ray_tracer::Intersection(1, shape);
      auto comps = ray_tracer::PrepareComputations(intersection, ray);
      CHECK(comps.IsInside() == true);
      CHECK(comps.GetNormalV() == ray_tracer::Vec3(0, 0, -1));
    }
  }
}
SCENARIO("7: Shading an intersection") {
  GIVEN("w = default_world(), r = ray(point(0, 0,-5), vector(0,0,1)), and"
        "shape = world_.Objects()[1], i = intersection(4,shape), when comps = prepare_computations(i,r)") {
    THEN("color = shade_hit(w, comps), color = color(0.38066, 0.47583, 0.2855)") {
      auto world = ray_tracer::World::DefaultWorld();
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
      auto shape = world.Objects()[0];
      auto intersection = ray_tracer::Intersection(4, shape);
      auto comps = ray_tracer::PrepareComputations(intersection, ray);
      auto color = ray_tracer::ShadeHit(world, comps);
      CHECK(utilities::Equal(color.GetRed(), 0.38066));
      CHECK(utilities::Equal(color.GetGreen(), 0.47583));
      CHECK(utilities::Equal(color.GetBlue(), 0.2855));
    }
  }
}

SCENARIO("7: Shading an intersection from the inside") {
  GIVEN("w = default_world(), r = ray(point(0,0,0), vector(0,0,1)), and shape = world_.getObject[1], i = intersection(0.5,shape), when comps = prepare_computations(i,r)") {
    THEN("color = shade_hit(w, comps), color = color(0.90498, 0.90498, 0.90498)") {
      auto world = ray_tracer::World::DefaultWorld();
      world.ChangeDefaultLight(PointLight(Point(0, 0.25, 0), Color(1, 1, 1)));
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, 0), ray_tracer::Vec3(0, 0, 1));
      auto shape = world.Objects()[1];
      auto intersection = ray_tracer::Intersection(0.5, shape);
      auto comps = ray_tracer::PrepareComputations(intersection, ray);
      auto color = ray_tracer::ShadeHit(world, comps);
      CHECK(comps.GetPoint() == ray_tracer::Point(0, 0, 0.5));
      CHECK(comps.GetEyeV() == ray_tracer::Vec3(0, 0, -1));
      CHECK(comps.IsInside() == true);
      CHECK(utilities::Equal(color.GetRed(), 0.90498));
      CHECK(utilities::Equal(color.GetGreen(), 0.90498));
      CHECK(utilities::Equal(color.GetBlue(), 0.90498));
    }
  }
}

SCENARIO("7: The color when a ray misses") {
  GIVEN("W = default world() And r = ray(point(0,.0,-5), vector(0,1,0))") {
    THEN("color = color_at(w, r), color = color(0,0,0)") {
      auto world = ray_tracer::World::DefaultWorld();
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 1, 0));
      auto color = ray_tracer::ColorAt(world, ray);
      CHECK(color == ray_tracer::Color(0, 0, 0));
    }
  }
}
SCENARIO("7: The color when a ray hits") {
  GIVEN("w = default_world(), r = ray(point(0,0,-5), vector(0,0,1))") {
    THEN("color = color_at(w, r), color = color(0.38066, 0.47583, 0.2855)") {
      auto world = ray_tracer::World::DefaultWorld();
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
      auto color = ray_tracer::ColorAt(world, ray);
      CHECK(color == ray_tracer::Color(0.38066, 0.47583, 0.2855));
    }
  }
}
SCENARIO("7: The color with an intersection behind the ray") {
  GIVEN("w = default_world(), outer = world_.Objects()[0], outer.set_material(ambient(1)), inner = world_.Objects()[1], inner.set_material(ambient(1)), r = ray(point(0,0,0.75), vector(0,0,-1))") {
    THEN("color = color_at(w, r), color = inner.material.color") {
      auto world = ray_tracer::World::DefaultWorld();
      auto outer = world.Objects()[0];
      auto outer_material = outer.GetMaterial();
      outer_material.SetAmbient(1);
      outer.SetMaterial(outer_material);
      world.UpdateObjects(0, outer);
      auto inner = world.Objects()[1];
      auto inner_material = inner.GetMaterial();
      inner_material.SetAmbient(1);
      inner.SetMaterial(inner_material);
      world.UpdateObjects(1, inner);
      auto ray = ray_tracer::Ray(ray_tracer::Point(0, 0, 0.75), ray_tracer::Vec3(0, 0, -1));
      auto color = ray_tracer::ColorAt(world, ray);
      CHECK(color == inner.GetMaterial().GetColor());
    }
  }
}
}  // namespace ray_tracer
