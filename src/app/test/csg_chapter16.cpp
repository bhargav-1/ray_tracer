#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>
#include "app/shapes/constructive_solid_geometry.h"
#include "app/shapes/cube.h"
#include "app/primitives/matrix.h"
#include "app/shapes/sphere.h"
#include "app/primitives/utilities.h"

namespace ray_tracer {
SCENARIO("16: CSG is created with an operation and two shapes") {
  GIVEN("left = sphere(), right = cube(), csg = csg(union, left, right)") {
    auto left = std::make_shared<Sphere>(1);
    auto right = std::make_shared<Cube>(2);
    auto csg_var = ConstructiveSolidGeometry::CreateCSG(1, left, right, CSGOperation::kUnion);
    THEN("csg.operation == union and csg.left == left and csg.right == right") {
      CHECK(csg_var->GetOperation() == CSGOperation::kUnion);
      CHECK(csg_var->GetLeft() == left);
      CHECK(csg_var->GetRight() == right);
      CHECK(left->GetParentGroup().lock() == csg_var);
      CHECK(right->GetParentGroup().lock() == csg_var);
    }
  }
}
SCENARIO("16: Evaluating the rule for a CSG operation") {
  GIVEN("result = intersection_allowed(<op>,<lhit>, <inl>, <inr>") {
    THEN("result = <result>") {
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, true, true, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, true, true, false) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, true, false, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, true, false, false) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, false, true, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, false, true, false) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, false, false, true) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kUnion, false, false, false) == true);

      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, true, true, true) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, true, true, false) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, true, false, true) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, true, false, false) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, false, true, true) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, false, true, false) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, false, false, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kIntersection, false, false, false) == false);

      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, true, true, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, true, true, false) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, true, false, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, true, false, false) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, false, true, true) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, false, true, false) == true);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, false, false, true) == false);
      CHECK(ConstructiveSolidGeometry::IntersectionAllowed(CSGOperation::kDifference, false, false, false) == false);
    }
  }
}

SCENARIO("16: Filtering a list of intersections") {
  GIVEN("csg = csg(<op>, sphere, cube), xs = intersextions(1:s1, 2:s2, 3:s1,4:s2), result= filter_intersections") {
    THEN("result.count =2, result[0] = xs[0], result[<x0>] = xs[<x1>]") {
      auto left = std::make_shared<Sphere>(1);
      auto right = std::make_shared<Cube>(2);
      auto csg_var1 = ConstructiveSolidGeometry::CreateCSG(1, left, right, CSGOperation::kUnion);
      auto csg_var2 = ConstructiveSolidGeometry::CreateCSG(2, left, right, CSGOperation::kIntersection);
      auto csg_var3 = ConstructiveSolidGeometry::CreateCSG(3, left, right, CSGOperation::kDifference);
      const std::vector<Intersection> xs_intersections = {
          Intersection(1, *left), Intersection(2, *right), Intersection(3, *left), Intersection(4, *right)};
      auto result1 = csg_var1->FilterIntersections(xs_intersections);
      CHECK(result1.size() == 2);
      CHECK(result1[0] == xs_intersections[0]);
      CHECK(result1[1] == xs_intersections[3]);
      auto result2 = csg_var2->FilterIntersections(xs_intersections);
      CHECK(result2.size() == 2);
      CHECK(result2[0] == xs_intersections[1]);
      CHECK(result2[1] == xs_intersections[2]);
      auto result3 = csg_var3->FilterIntersections(xs_intersections);
      CHECK(result3.size() == 2);
      CHECK(result3[0] == xs_intersections[0]);
      CHECK(result3[1] == xs_intersections[1]);
    }
  }
}
SCENARIO("16:A ray misses a CSG object") {
  GIVEN("csg = csg(union, sphere, cube), ray = ray(<origin>, <direction>)") {
    auto left = std::make_shared<Sphere>(1);
    auto right = std::make_shared<Cube>(2);
    auto csg_var = ConstructiveSolidGeometry::CreateCSG(1, left, right, CSGOperation::kUnion);
    THEN("csg.intersect(ray) == empty") {
      auto ray_var1 = Ray(Point(0, 2, -5), Vec3(0, 0, 1).Normalize());
      auto result1 = csg_var->Intersect(ray_var1);
      CHECK(result1.empty());
      auto ray_var2 = Ray(Point(0, 2, -5), Vec3(0, 0, 1).Normalize());
      auto result2 = csg_var->Intersect(ray_var2);
      CHECK(result2.empty());
    }
  }
}

SCENARIO("16: A ray hits a CSG object") {
  GIVEN("csg = csg(union, sphere, cube), ray = ray(<origin>, <direction>)") {
    auto sphere1 = std::make_shared<Sphere>(1);
    auto sphere2 = std::make_shared<Sphere>(2);
    sphere2->SetTransform(Matrix<4, 4>::Translation(0, 0, 0.5));
    auto csg_var = ConstructiveSolidGeometry::CreateCSG(1, sphere1, sphere2, CSGOperation::kUnion);
    THEN("csg.intersect(ray) == xs") {
      auto ray_var = Ray(Point(0, 0, -5), Vec3(0, 0, 1).Normalize());
      auto result = csg_var->LocalIntersect(ray_var);
      CHECK(result.size() == 2);
      CHECK(utilities::Equal(result[0].GetTime(), 4.0));
      CHECK(result[0].GetObject() == sphere1.get());
      CHECK(utilities::Equal(result[1].GetTime(), 6.5));
      CHECK(result[1].GetObject() == sphere2.get());
    }
  }
}
}  // namespace ray_tracer