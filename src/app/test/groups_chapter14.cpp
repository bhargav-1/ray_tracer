#include <catch2/catch_test_macros.hpp>

// #include "app/cylinder.h"
#include <cmath>
#include <memory>
#include <numbers>
#include "app/group.h"
#include "app/matrix.h"
#include "app/shape.h"
#include "app/sphere.h"

namespace ray_tracer {
SCENARIO("14: Creating a new group") {
  GIVEN("G = Group()") {
    THEN("G.transform == identity_matrix()") {
      auto group_var = Group(1);
      CHECK(group_var.GetTransform() == Matrix<4, 4>::IdentityMatrix());
    }
  }
}
SCENARIO("14: A shape has a parent att") {
  GIVEN("s = tes_shape()") {
    THEN("s.parent == null") {
      auto shape_var = std::make_shared<Sphere>(1);
      CHECK(shape_var->GetParentGroup().lock() == nullptr);
    }
  }
}
SCENARIO("14: adding a child to a group") {
  GIVEN("G = Group(), s= sphere, g.add(s)") {
    THEN("G.children includes s, s.parent == G") {
      auto group_var = std::make_shared<Group>(1);
      auto shape_var = std::make_shared<Sphere>(1);
      group_var->AddChild(shape_var);
      CHECK(group_var->GetChildren().size() == 1);
      CHECK(group_var->GetChildren().front() == shape_var);
      CHECK(shape_var->GetParentGroup().lock() == group_var);
    }
  }
}
SCENARIO("14: intersecting a ray with empty group") {
  GIVEN("G = Group(), r = ray(point(0,0,0), vector(0,0,1))") {
    THEN("xs = G.intersect(r) is empty") {
      auto group_var = std::make_shared<Group>(1);
      auto ray_var = Ray(Point(0, 0, 0), Vec3(0, 0, 1));
      auto intersect_xs = group_var->Intersect(ray_var);
      CHECK(intersect_xs.empty());
    }
  }
}
SCENARIO("14: Intersecting a ray with non empty group") {
  GIVEN("G = Group(), s = sphere, g.add(s), r = ray(point(0,0,-5), vector(0,0,1))") {
    THEN("xs = G.intersect(r)") {
      auto group_var = std::make_shared<Group>(1);
      auto sphere_1 = std::make_shared<Sphere>(1);
      group_var->AddChild(sphere_1);
      auto sphere_2 = std::make_shared<Sphere>(1);
      sphere_2->SetTransform(Matrix<4, 4>::Translation(0, 0, -3));
      auto sphere_3 = std::make_shared<Sphere>(1);
      sphere_3->SetTransform(Matrix<4, 4>::Translation(5, 0, 0));
      group_var->AddChild(sphere_2);
      group_var->AddChild(sphere_3);

      auto ray_var = Ray(Point(0, 0, -5), Vec3(0, 0, 1));
      auto intersect_xs = group_var->Intersect(ray_var);
      CHECK(intersect_xs.size() == 4);
      CHECK(intersect_xs[0].GetObject() == sphere_2.get());
      CHECK(intersect_xs[1].GetObject() == sphere_2.get());
      CHECK(intersect_xs[2].GetObject() == sphere_1.get());
      CHECK(intersect_xs[3].GetObject() == sphere_1.get());
    }
  }
}
SCENARIO("14:Intesecting a transformed group") {
  GIVEN(
      "group_var = std::make_shared<Group>(1), settransform(g, scale 2, 2,2 ),s=sphere.translation(5,0, "
      "0),g.add(s),r=(10,0,-10),v(0,0,1)xs = intersect(g,r)") {
    THEN("xs.count=2") {
      auto group_var = std::make_shared<Group>(1);
      group_var->SetTransform(Matrix<4, 4>::Scaling(2, 2, 2));
      auto sphere_1 = std::make_shared<Sphere>(1);
      sphere_1->SetTransform(Matrix<4, 4>::Translation(5, 0, 0));
      group_var->AddChild(sphere_1);
      auto ray_var = Ray(Point(10, 0, -10), Vec3(0, 0, 1));
      auto intersect_xs = group_var->Intersect(ray_var);
      CHECK(intersect_xs.size() == 2);
    }
  }
}
SCENARIO("14: Converting a point from world to object space") {
  GIVEN("g1 = group, t(r_y([pi/2]), g2.t(scal(2,2,2)), s.t(translate(5,0,0), g2.add(s) g1.add(g2)") {
    THEN("p = s.world_to_obect(-2,0,-10), p = point(0,0,-1)") {
      auto group_var = std::make_shared<Group>(1);
      group_var->SetTransform(Matrix<4, 4>::RotationY(std::numbers::pi / 2));
      auto group_2 = std::make_shared<Group>(2);
      group_2->SetTransform(Matrix<4, 4>::Scaling(2, 2, 2));
      group_var->AddChild(group_2);
      auto sphere_1 = std::make_shared<Sphere>(1);
      sphere_1->SetTransform(Matrix<4, 4>::Translation(5, 0, 0));
      group_2->AddChild(sphere_1);
      auto point_var = sphere_1->WorldToObject(Point(-2, 0, -10));
      CHECK(point_var == Point(0, 0, -1));
    }
  }
}
SCENARIO("Converting a normal from object to world space") {
  GIVEN("g1 = group, t(r_y([pi/2]), g2.t(scal(1,2,3)), s.t(translate(5,0,0), g2.add(s) g1.add(g2)") {
    THEN("n = s.normal_to_world(point(sqrt(3)/3, sqrt(3)/3, sqrt(3)/3)), n = vector(0.2857, 0.4285, -0.8571)") {
      auto group_var = std::make_shared<Group>(1);
      group_var->SetTransform(Matrix<4, 4>::RotationY(std::numbers::pi / 2));
      auto group_2 = std::make_shared<Group>(2);
      group_2->SetTransform(Matrix<4, 4>::Scaling(1, 2, 3));
      group_var->AddChild(group_2);
      auto sphere_1 = std::make_shared<Sphere>(1);
      sphere_1->SetTransform(Matrix<4, 4>::Translation(5, 0, 0));
      group_2->AddChild(sphere_1);
      auto normal_var =
          sphere_1->NormalToWorld(Vec3(std::numbers::sqrt3 / 3, std::numbers::sqrt3 / 3, std::numbers::sqrt3 / 3));
      CHECK(normal_var == Vec3(0.2857, 0.4285, -0.8571));
    }
  }
}
SCENARIO("14: Finding the normal on a child object") {
  GIVEN("g1 = group, t(r_y([pi/2]), g2.t(scal(1,2,3)), s.t(translate(5,0,0), g2.add(s) g1.add(g2)") {
    THEN("n = s.normal_to_world(point(sqrt(3)/3, sqrt(3)/3, sqrt(3)/3)), n = vector(0.2857, 0.4285, -0.8571)") {
      auto group_var = std::make_shared<Group>(1);
      group_var->SetTransform(Matrix<4, 4>::RotationY(std::numbers::pi / 2));
      auto group_2 = std::make_shared<Group>(2);
      group_2->SetTransform(Matrix<4, 4>::Scaling(1, 2, 3));
      group_var->AddChild(group_2);
      auto sphere_1 = std::make_shared<Sphere>(1);
      sphere_1->SetTransform(Matrix<4, 4>::Translation(5, 0, 0));
      group_2->AddChild(sphere_1);
      auto normal_var = sphere_1->NormalAt(Point(1.7321, 1.1547, -5.5774)); // NOLINT
      CHECK(normal_var == Vec3(0.2857, 0.4285, -0.8571));
    }
  }
}
}  // namespace ray_tracer