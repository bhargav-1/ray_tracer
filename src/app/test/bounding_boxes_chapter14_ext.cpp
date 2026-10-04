#include <catch2/catch_test_macros.hpp>
#include <limits>

#include <cmath>
#include <cstddef>
#include <memory>
#include <numbers>
#include <optional>
#include <vector>
#include "app/bounding_box.h"
#include "app/cone.h"
#include "app/cube.h"
#include "app/cylinder.h"
#include "app/group.h"
#include "app/intersection.h"
#include "app/matrix.h"
#include "app/plane.h"
#include "app/point.h"
#include "app/ray.h"
#include "app/shape.h"
#include "app/sphere.h"
#include "app/vec3.h"

namespace ray_tracer {
class TestShape : public Shape {
 public:
  explicit TestShape(const std::size_t shape_id) : Shape(shape_id) {}
  ~TestShape() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override {
    saved_ray_ = ray;
    return {};
  };
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override {
    return {point.GetX(), point.GetY(), point.GetZ()};
  };
  auto BoundsOf() const -> BoundingBox override { return {Point(-1, -1, -1), Point(1, 1, 1)}; }
  [[nodiscard]] auto GetSavedRay() const -> std::optional<Ray> { return saved_ray_; }

 private:
  mutable std::optional<Ray> saved_ray_;
};

SCENARIO("14_ext: Creating an empty bb") {
  GIVEN("bb = BoundingBox()") {
    THEN("bb.min == point(inf, inf, inf) and bb.max == point(-inf, -inf, -inf)") {
      auto bbox = BoundingBox();
      CHECK(bbox.GetMin() == Point(std::numeric_limits<double>::infinity(),
                                   std::numeric_limits<double>::infinity(),
                                   std::numeric_limits<double>::infinity()));
      CHECK(bbox.GetMax() == Point(-1 * std::numeric_limits<double>::infinity(),
                                   -1 * std::numeric_limits<double>::infinity(),
                                   -1 * std::numeric_limits<double>::infinity()));
    }
  }
}
SCENARIO("14_ext:Creating a bb with volume") {
  GIVEN("bb = BoundingBox(point(-1, -2, -3), point(3, 2, 1))") {
    THEN("bb.min == point(-1, -2, -3) and bb.max == point(3, 2, 1)") {
      auto bbox = BoundingBox(Point(-1, -2, -3), Point(3, 2, 1));
      CHECK(bbox.GetMin() == Point(-1, -2, -3));
      CHECK(bbox.GetMax() == Point(3, 2, 1));
    }
  }
}

SCENARIO("14_ext: Adding points to an empty bb") {
  GIVEN("box = bounding_box(); p1= point(-5,2,0), p2= Point(7,0,-3), box.add(p1,p2)") {
    THEN("box.min = point(-5,2,-3) and box.max = point(7,2,0)") {
      auto box = BoundingBox();
      auto point_p1 = Point(-5, 2, 0);
      auto point_p2 = Point(7, 0, -3);
      box.AddPoint(point_p1);
      box.AddPoint(point_p2);
      CHECK(box.GetMin() == Point(-5, 0, -3));
      CHECK(box.GetMax() == Point(7, 2, 0));
    }
  }
}

SCENARIO("14_ext: A sphere has a bounding box") {
  GIVEN("s = sphere(), box = s.bounds()") {
    THEN("box.min == point(-1, -1, -1) and box.max == point(1, 1, 1)") {
      auto sphere_var = std::make_shared<Sphere>(1);
      auto box = sphere_var->BoundsOf();
      CHECK(box.GetMin() == Point(-1, -1, -1));
      CHECK(box.GetMax() == Point(1, 1, 1));
    }
  }
}
SCENARIO("14_ext: A plane has a bounding box") {
  GIVEN("p = plane(), box = p.bounds()") {
    THEN("box.min == point(-inf, 0, -inf) and box.max == point(inf, 0, inf)") {
      auto plane_var = std::make_shared<Plane>(1);
      auto box = plane_var->BoundsOf();
      CHECK(box.GetMin() ==
            Point(-1 * std::numeric_limits<double>::infinity(), 0, -1 * std::numeric_limits<double>::infinity()));
      CHECK(box.GetMax() == Point(std::numeric_limits<double>::infinity(), 0, std::numeric_limits<double>::infinity()));
    }
  }
}
SCENARIO("14_ext: A cube has a bounding box") {
  GIVEN("c = cube(), box = c.bounds()") {
    THEN("box.min == point(-1, -1, -1) and box.max == point(1, 1, 1)") {
      auto cube_var = std::make_shared<Cube>(1);
      auto box = cube_var->BoundsOf();
      CHECK(box.GetMin() == Point(-1, -1, -1));
      CHECK(box.GetMax() == Point(1, 1, 1));
    }
  }
}

SCENARIO("14_ext: A cylinder has a bounding box") {
  GIVEN("cyl = cylinder(), box = cyl.bounds()") {
    THEN("box.min == point(-1, -inf, -1) and box.max == point(1, inf, 1)") {
      auto cyl_var = std::make_shared<Cylinder>(1);
      auto box = cyl_var->BoundsOf();
      CHECK(box.GetMin() == Point(-1, -1 * std::numeric_limits<double>::infinity(), -1));
      CHECK(box.GetMax() == Point(1, std::numeric_limits<double>::infinity(), 1));
    }
  }
}
SCENARIO("14_ext: A bounded cyclinder has a bounding box") {
  GIVEN("cyl = cylinder(), cyl.minimum = -5, cyl.maximum = 3, box = cyl.bounds()") {
    THEN("box.min == point(-1, -5, -1) and box.max == point(1, 3, 1)") {
      auto cyl_var = std::make_shared<Cylinder>(1);
      cyl_var->SetMinimum(-5);
      cyl_var->SetMaximum(3);
      auto box = cyl_var->BoundsOf();
      CHECK(box.GetMin() == Point(-1, -5, -1));
      CHECK(box.GetMax() == Point(1, 3, 1));
    }
  }
}
SCENARIO("14_ext: A cone has a bounding box") {
  GIVEN("cone = cone(), box = cone.bounds()") {
    THEN("box.min == point(-inf, -inf, -inf) and box.max == point(inf, inf, inf)") {
      auto cone_var = std::make_shared<Cone>(1);
      auto box = cone_var->BoundsOf();
      CHECK(box.GetMin() == Point(-1 * std::numeric_limits<double>::infinity(),
                                  -1 * std::numeric_limits<double>::infinity(),
                                  -1 * std::numeric_limits<double>::infinity()));
      CHECK(box.GetMax() == Point(std::numeric_limits<double>::infinity(),
                                  std::numeric_limits<double>::infinity(),
                                  std::numeric_limits<double>::infinity()));
    }
  }
}
SCENARIO("14_ext: A bounded cone has a bounding box") {
  GIVEN("cone = cone(), cone.minimum = -5, cone.maximum = 3, box = cone.bounds()") {
    THEN("box.min == point(-5, -5, -5) and box.max == point(5, 3, 5)") {
      auto cone_var = std::make_shared<Cone>(1);
      cone_var->SetMinimum(-5);
      cone_var->SetMaximum(3);
      auto box = cone_var->BoundsOf();
      CHECK(box.GetMin() == Point(-5, -5, -5));
      CHECK(box.GetMax() == Point(5, 3, 5));
    }
  }
}

// SCENARIO("14_ext: A triangle has a bb") {
//   GIVEN("p1 = -3,7,2, p2 = 6,2, -4, p3 = 2, -1, 4, triangle = triangle(p1,p2,p3), box = triangle.bounds()") {
//     THEN("box.min == point(-3, -1, -4) and box.max == point(6, 7, 4)") {
//       auto triangle_var = std::make_shared<Triangle>(Point(-3, 7, 2), Point(6, 2, -4), Point(2, -1, 4), 1);
//       auto box = triangle_var->BoundsOf();
//       CHECK(box.GetMin() == Point(-3, -1, -4));
//       CHECK(box.GetMax() == Point(6, 7, 4));
//     }
//   }
// }

SCENARIO("14_ext: Test shape has a bounding box") {
  GIVEN("test_shape = test_shape(), box = test_shape.bounds()") {
    THEN("box.min == point(-1, -1, -1) and box.max == point(1, 1, 1)") {
      auto test_shape_var = std::make_shared<TestShape>(1);
      auto box = test_shape_var->BoundsOf();
      CHECK(box.GetMin() == Point(-1, -1, -1));
      CHECK(box.GetMax() == Point(1, 1, 1));
    }
  }
}
SCENARIO("14_ext : Adding one bounding box to another") {
  GIVEN(
      "box1 = bounding_box(point(-5, -2, 0), point(7, 4, 4)), box2 = bounding_box(point(8, -7, -2), point(14, 2, 8))") {
    THEN("box1.add(box2) == bounding_box(point(-5, -7, -2), point(14, 4, 8))") {
      auto box1 = BoundingBox(Point(-5, -2, 0), Point(7, 4, 4));
      auto box2 = BoundingBox(Point(8, -7, -2), Point(14, 2, 8));
      box1 = box1 + box2;
      CHECK(box1.GetMin() == Point(-5, -7, -2));
      CHECK(box1.GetMax() == Point(14, 4, 8));
    }
  }
}
SCENARIO("14_ext: Checking to see if a box contains a given point") {
  GIVEN("box = bounding_box(point(-5, -2, 0), point(11, 4, 7)), p1(5,-2,0), p2(11,4,7), p3(8,1,3), p4(3,0,3)...") {
    THEN("box.contains(p1) == false, box.contains(p2) == true, box.contains(p3) == false") {
      auto box = BoundingBox(Point(5, -2, 0), Point(11, 4, 7));
      auto point1 = Point(5, -2, 0);
      auto point2 = Point(11, 4, 7);
      auto point3 = Point(8, 1, 3);
      CHECK(box.Contains(point1) == true);
      CHECK(box.Contains(point2) == true);
      CHECK(box.Contains(point3) == true);
      CHECK(box.Contains(Point(3, 0, 3)) == false);
      CHECK(box.Contains(Point(8, -4, 3)) == false);
      CHECK(box.Contains(Point(8, 1, -1)) == false);
      CHECK(box.Contains(Point(13, 1, 3)) == false);
      CHECK(box.Contains(Point(8, 5, 3)) == false);
      CHECK(box.Contains(Point(8, 1, 8)) == false);
    }
  }
}

SCENARIO("14_ext: Checking to see if a box contains a given box") {
  GIVEN("box = bb((5,-2,0),(11,4,7), bb2 = bb(min, max))") {
    THEN("bb.contains(bb2) ==result") {
      auto box1 = BoundingBox(Point(5, -2, 0), Point(11, 4, 7));
      auto box2 = BoundingBox(Point(5, -2, 0), Point(11, 4, 7));
      CHECK(box1.Contains(box2) == true);
      auto box3 = BoundingBox(Point(6, -1, 1), Point(10, 3, 6));
      auto box4 = BoundingBox(Point(4, -3, -1), Point(10, 3, 6));
      auto box5 = BoundingBox(Point(6, -1, 1), Point(12, 5, 8));
      CHECK(box1.Contains(box3) == true);
      CHECK(box1.Contains(box4) == false);
      CHECK(box1.Contains(box5) == false);
    }
  }
}
SCENARIO("14_ext: Transforming a bb") {
  GIVEN("bb((-1,-1,1), (1,1,1)), transform = rotx(pi/4)*rot_y(pi/4)") {
    THEN("bb.transform(transform) == bb(min, max)") {
      auto box = BoundingBox(Point(-1, -1, -1), Point(1, 1, 1));
      const auto transform =
          Matrix<4, 4>::RotationX(std::numbers::pi / 4) * Matrix<4, 4>::RotationY(std::numbers::pi / 4);
      auto transformed_box = box.Transform(transform);
      CHECK(transformed_box.GetMin() == Point(-1.41421, -1.70711, -1.70711));
      CHECK(transformed_box.GetMax() == Point(1.41421, 1.70711, 1.70711));
    }
  }
}
SCENARIO("14_ext: Querying a shape's bb in its parent space") {
  GIVEN("shape =sphere(), shape.transform = translation(1, -3, 5)*scale(0.5,2,4), bb =parent_space_boundsof(shape)") {
    THEN("bb.min == point(0.5, -5, 1) and bb.max == point(1.5, -1, 9)") {
      auto sphere_var = std::make_shared<Sphere>(1);
      sphere_var->SetTransform(Matrix<4, 4>::Translation(1, -3, 5) * Matrix<4, 4>::Scaling(0.5, 2, 4));
      auto box = sphere_var->ParentSpaceBounds();
      CHECK(box.GetMin() == Point(0.5, -5, 1));
      CHECK(box.GetMax() == Point(1.5, -1, 9));
    }
  }
}
SCENARIO("14_ext: A group has a bounding box that contains its children") {
  GIVEN(
      "s = sphere, s.transform = translation(2, 5,-3)*scale(2,2,2), cylinder, cyl.min=-2, max=2, cyl.transform = "
      "translation(-4,-1,4)*scale(0.5,1,0.5), group = group(), group.add(s), group.add(cyl), bb = group.bounds()") {
    THEN("bb.min == point(-4, -3, -5) and bb.max == point(4, 7, 4)") {
      auto sphere_var = std::make_shared<Sphere>(1);
      sphere_var->SetTransform(Matrix<4, 4>::Translation(2, 5, -3) * Matrix<4, 4>::Scaling(2, 2, 2));
      auto cylinder_var = std::make_shared<Cylinder>(2);
      cylinder_var->SetMinimum(-2);
      cylinder_var->SetMaximum(2);
      cylinder_var->SetTransform(Matrix<4, 4>::Translation(-4, -1, 4) * Matrix<4, 4>::Scaling(0.5, 1, 0.5));
      auto group_var = std::make_shared<Group>(3);
      group_var->AddChild(sphere_var);
      group_var->AddChild(cylinder_var);
      auto box = group_var->BoundsOf();
      CHECK(box.GetMin() == Point(-4.5, -3, -5));
      CHECK(box.GetMax() == Point(4, 7, 4.5));
    }
  }
}

// SCENARIO("14_ext: CSG group has a bb that contains its children") {
//   GIVEN("left= sphere(1),right=sphere(2),right.translation(2,3,4), csg(diff, left, right), bb = csg.bounds()") {
//     THEN("bb.min == point(-1, -1, -1) and bb.max == point(4, 5, 6)") {
//       auto left = std::make_shared<Sphere>(1);
//       auto right = std::make_shared<Sphere>(2);
//       right->SetTransform(Matrix<4, 4>::Translation(2, 3, 4));
//       auto csg_var = std::make_shared<CSG>(CSGOperation::Difference, left, right);
//       auto box = csg_var->BoundsOf();
//       CHECK(box.GetMin() == Point(-1, -1, -1));
//       CHECK(box.GetMax() == Point(4, 5, 6));
//     }
//   }
// }
SCENARIO("14_ext: Intersecting a ray with bb at the origin") {
  GIVEN("bb = (p(-1,-1,-1),p(1,1,1), dir= normalize(<dir>), r= ray(<origin>, dir)") {
    THEN("bb.intersect(r) == result") {
      auto box = BoundingBox(Point(-1, -1, -1), Point(1, 1, 1));
      auto result1 = box.Intersects(Ray(Point(5, 0.5, 0), Vec3(-1, 0, 0).Normalize()));
      CHECK(result1 == true);
      auto result2 = box.Intersects(Ray(Point(-5, 0.5, 0), Vec3(1, 0, 0).Normalize()));
      CHECK(result2 == true);
      auto result3 = box.Intersects(Ray(Point(0.5, 5, 0), Vec3(0, -1, 0).Normalize()));
      CHECK(result3 == true);
      auto result4 = box.Intersects(Ray(Point(0.5, -5, 0), Vec3(0, 1, 0).Normalize()));
      CHECK(result4 == true);
      auto result5 = box.Intersects(Ray(Point(0.5, 0, 5), Vec3(0, 0, -1).Normalize()));
      CHECK(result5 == true);
      auto result6 = box.Intersects(Ray(Point(0.5, 0, -5), Vec3(0, 0, 1).Normalize()));
      CHECK(result6 == true);
      auto result7 = box.Intersects(Ray(Point(0, 0.5, 0), Vec3(0, 0, 1).Normalize()));
      CHECK(result7 == true);
      auto result8 = box.Intersects(Ray(Point(-2, 0, 0), Vec3(2, 4, 6).Normalize()));
      CHECK(result8 == false);
      auto result9 = box.Intersects(Ray(Point(0, -2, 0), Vec3(6, 2, 4).Normalize()));
      CHECK(result9 == false);
      auto result10 = box.Intersects(Ray(Point(0, 0, -2), Vec3(4, 6, 2).Normalize()));
      CHECK(result10 == false);
      auto result11 = box.Intersects(Ray(Point(2, 0, 2), Vec3(0, 0, -1).Normalize()));
      CHECK(result11 == false);
      auto result12 = box.Intersects(Ray(Point(0, 2, 2), Vec3(0, -1, 0).Normalize()));
      CHECK(result12 == false);
      auto result13 = box.Intersects(Ray(Point(2, 2, 0), Vec3(-1, 0, 0).Normalize()));
      CHECK(result13 == false);
    }
  }
}
SCENARIO("14_ext: Intersecting a ray with a non-cubic bb") {
  GIVEN("bb = ((5, -2,0), (11,4,7)), dir= normalize(<dir>), r= ray(<origin>, dir)") {
    THEN("bb.intersect(r) == result") {
      auto box = BoundingBox(Point(5, -2, 0), Point(11, 4, 7));
      auto result1 = box.Intersects(Ray(Point(15, 1, 2), Vec3(-1, 0, 0).Normalize()));
      CHECK(result1 == true);
      auto result2 = box.Intersects(Ray(Point(-5, 1, 4), Vec3(1, 0, 0).Normalize()));
      CHECK(result2 == true);
      auto result3 = box.Intersects(Ray(Point(7, 6, 5), Vec3(0, -1, 0).Normalize()));
      CHECK(result3 == true);
      auto result4 = box.Intersects(Ray(Point(9, -5, 6), Vec3(0, 1, 0).Normalize()));
      CHECK(result4 == true);
      auto result5 = box.Intersects(Ray(Point(8, 2, 12), Vec3(0, 0, -1).Normalize()));
      CHECK(result5 == true);
      auto result6 = box.Intersects(Ray(Point(6, 0, -5), Vec3(0, 0, 1).Normalize()));
      CHECK(result6 == true);
      auto result7 = box.Intersects(Ray(Point(8, 1, 3.5), Vec3(0, 0, 1).Normalize()));
      CHECK(result7 == true);
      auto result8 = box.Intersects(Ray(Point(9, -1, -8), Vec3(2, 4, 6).Normalize()));
      CHECK(result8 == false);
      auto result9 = box.Intersects(Ray(Point(8, 3, -4), Vec3(6, 2, 4).Normalize()));
      CHECK(result9 == false);
      auto result10 = box.Intersects(Ray(Point(9, -1, -2), Vec3(4, 6, 2).Normalize()));
      CHECK(result10 == false);
      auto result11 = box.Intersects(Ray(Point(4, 0, 9), Vec3(0, 0, -1).Normalize()));
      CHECK(result11 == false);
      auto result12 = box.Intersects(Ray(Point(8, 6, -1), Vec3(0, -1, 0).Normalize()));
      CHECK(result12 == false);
      auto result13 = box.Intersects(Ray(Point(12, 5, 4), Vec3(-1, 0, 0).Normalize()));
      CHECK(result13 == false);
    }
  }
}
SCENARIO("14_ext: Intersecting ray+group doesn't test children if box is missed") {
  GIVEN("child =test_shape(), shape=group();shape.add(child),r=ray(point(0,0,-5),vec(0,1,0))") {
    THEN("intersect(r) == 0") {
      auto child = std::make_shared<TestShape>(1);
      auto shape = std::make_shared<Group>(2);
      shape->AddChild(child);
      auto ray_var = Ray(Point(0, 0, -5), Vec3(0, 1, 0));
      auto intesect_xs = shape->Intersect(ray_var);
      CHECK(child->GetSavedRay().has_value() == false);
    }
  }
}

SCENARIO("14_ext: Intersecting ray+group does test children if box is hit") {
  GIVEN("child =test_shape(), shape=group();shape.add(child),r=ray(point(0,0,-5),vec(0,0,1))") {
    THEN("intersect(r) == 0") {
      auto child = std::make_shared<TestShape>(1);
      auto shape = std::make_shared<Group>(2);
      shape->AddChild(child);
      auto ray_var = Ray(Point(0, 0, -5), Vec3(0, 0, 1));
      auto intesect_xs = shape->Intersect(ray_var);
      CHECK(child->GetSavedRay().has_value() == true);
    }
  }
}
// SCENARIO("14_ext: Interseting ray+csg doesnot test children if box is missed") {
//   GIVEN("left = test_shape(), right = test_shape(), csg = csg(diff, left, right), r=ray(point(0,0,-5),vec(0,1,0))") {
//     THEN("intersect(r) == 0") {
//       auto left = std::make_shared<TestShape>(1);
//       auto right = std::make_shared<TestShape>(2);
//       auto csg_var = std::make_shared<CSG>(CSGOperation::Difference, left, right);
//       auto ray_var = Ray(Point(0, 0, -5), Vec3(0, 1, 0));
//       auto intesect_xs = csg_var->Intersect(ray_var);
//       CHECK(left->GetSavedRay().has_value() == false);
//       CHECK(right->GetSavedRay().has_value() == false);
//     }
//   }
// }
// SCENARIO("14_ext: Intersecting ray+csg does test children if box is hit") {
//   GIVEN("left = test_shape(), right = test_shape(), csg = csg(diff, left, right), r=ray(point(0,0,-5),vec(0,0,1))") {
//     THEN("intersect(r) == 0") {
//       auto left = std::make_shared<TestShape>(1);
//       auto right = std::make_shared<TestShape>(2);
//       auto csg_var = std::make_shared<CSG>(CSGOperation::Difference, left, right);
//       auto ray_var = Ray(Point(0, 0, -5), Vec3(0, 0, 1));
//       auto intesect_xs = csg_var->Intersect(ray_var);
//       CHECK(left->GetSavedRay().has_value() == true);
//       CHECK(right->GetSavedRay().has_value() == true);
//     }
//   }
// }

SCENARIO("14_ext: Splitting a perfect cube") {
  GIVEN("box = bb((-1,-4,-5), (9,6,5)), left, right = box.split()") {
    THEN(
        "left.min == point(-1,-4,-5) and left.max == point(4,6,5) and right.min == point(4,-4,-5) and right.max == "
        "point(9,6,5)") {
      auto box = BoundingBox(Point(-1, -4, -5), Point(9, 6, 5));
      auto [left, right] = box.SplitBounds();
      CHECK(left.GetMin() == Point(-1, -4, -5));
      CHECK(left.GetMax() == Point(4, 6, 5));
      CHECK(right.GetMin() == Point(4, -4, -5));
      CHECK(right.GetMax() == Point(9, 6, 5));
    }
  }
}
SCENARIO("14_ext: splitting an x_wide box") {
  GIVEN("box= bb((-1,-2,-3),(9,5.5,3)), left, right = box.split()") {
    THEN(
        "left.min == point(-1,-2,-3) and left.max == point(4,5.5,3) and right.min == point(4,-2,-3) and right.max == "
        "point(9,5.5,3)") {
      auto box = BoundingBox(Point(-1, -2, -3), Point(9, 5.5, 3));
      auto [left, right] = box.SplitBounds();
      CHECK(left.GetMin() == Point(-1, -2, -3));
      CHECK(left.GetMax() == Point(4, 5.5, 3));
      CHECK(right.GetMin() == Point(4, -2, -3));
      CHECK(right.GetMax() == Point(9, 5.5, 3));
    }
  }
}
SCENARIO("14_ext: splitting a y_wide box") {
  GIVEN("box= bb((-1,-2,-3),(5,8,3)), left, right = box.split()") {
    THEN(
        "left.min == point(-1,-2,-3) and left.max == point(5,3,3) and right.min == point(-1,3,-3) and right.max "
        "== point(5,8,3)") {
      auto box = BoundingBox(Point(-1, -2, -3), Point(5, 8, 3));
      auto [left, right] = box.SplitBounds();
      CHECK(left.GetMin() == Point(-1, -2, -3));
      CHECK(left.GetMax() == Point(5, 3, 3));
      CHECK(right.GetMin() == Point(-1, 3, -3));
      CHECK(right.GetMax() == Point(5, 8, 3));
    }
  }
}
SCENARIO("14_ext: splitting a z_wide box") {
  GIVEN("box= bb((-1,-2,-3),(5,3,7)), left, right = box.split()") {
    THEN(
        "left.min == point(-1,-2,-3) and left.max == point(5,3,2) and right.min == point(-1,-2,2) and right.max "
        "== point(5,3,7)") {
      auto box = BoundingBox(Point(-1, -2, -3), Point(5, 3, 7));
      auto [left, right] = box.SplitBounds();
      CHECK(left.GetMin() == Point(-1, -2, -3));
      CHECK(left.GetMax() == Point(5, 3, 2));
      CHECK(right.GetMin() == Point(-1, -2, 2));
      CHECK(right.GetMax() == Point(5, 3, 7));
    }
  }
}
SCENARIO("14_ext: Partitioning a group's children") {
  GIVEN(
      "s1 = sphere(1), s2 = sphere(2), s3 = sphere(3), s1.transform=translation(-2,0,0), "
      "s2.transform=translation(2,0,0), group=group(), group.add(s1,s2,s3)") {
    THEN("left, right = group.partition_children()") {
      auto s1_sphere = std::make_shared<Sphere>(1);
      auto s2_sphere = std::make_shared<Sphere>(2);
      auto s3_sphere = std::make_shared<Sphere>(3);
      s1_sphere->SetTransform(Matrix<4, 4>::Translation(-2, 0, 0));
      s2_sphere->SetTransform(Matrix<4, 4>::Translation(2, 0, 0));
      auto group_var = std::make_shared<Group>(4);
      group_var->AddChild(s1_sphere);
      group_var->AddChild(s2_sphere);
      group_var->AddChild(s3_sphere);
      auto [left, right] = group_var->PartitionChildren();
      CHECK(group_var->GetChildren() == std::vector<std::shared_ptr<Shape>>{s3_sphere});
      CHECK(left == std::vector<std::shared_ptr<Shape>>{s1_sphere});
      CHECK(right == std::vector<std::shared_ptr<Shape>>{s2_sphere});
    }
  }
}
SCENARIO("14_ext: Creating a sub-group from a list of children") {
  GIVEN("s1 = sphere(1), s2 = sphere(2), s3 = sphere(3), children = [s1,s2], group=group()") {
    THEN("sub_group = group.make_sub_group(children)") {
      auto s1_sphere = std::make_shared<Sphere>(1);
      auto s2_sphere = std::make_shared<Sphere>(2);
      auto s3_sphere = std::make_shared<Sphere>(3);
      const std::vector<std::shared_ptr<Shape>> children{s1_sphere, s2_sphere};
      auto group_var = std::make_shared<Group>(5);
      auto sub_group = group_var->MakeSubGroup(children);
      const auto group_children = group_var->GetChildren();
      REQUIRE(group_children.size() == 1);
      CHECK(std::dynamic_pointer_cast<Group>(group_children[0]) == sub_group);
      CHECK(sub_group->GetParentGroup().lock() == group_var);
      REQUIRE(sub_group->GetChildren().size() == 2);
      CHECK(sub_group->GetChildren()[0] == s1_sphere);
      CHECK(sub_group->GetChildren()[1] == s2_sphere);
      CHECK(s1_sphere->GetParentGroup().lock() == sub_group);
      CHECK(s2_sphere->GetParentGroup().lock() == sub_group);
    }
  }
}
SCENARIO("14_ext: Subdividing a primitive does nothing") {
  GIVEN("s = sphere(1), s.subdivide()") {
    THEN("s is unchanged") {
      auto shape = std::make_shared<Sphere>(1);
      shape->Divide(1);
      CHECK(*shape == Sphere(1));
    }
  }
}
SCENARIO("14_ext: Subdividing a group partitions its children") {
  GIVEN(
      "s1 = sphere(1), s2 = sphere(2), s3 = sphere(3), s1.transform=translation(-2,-2,0), "
      "s2.transform=translation(-2,2,0),s3.scale(4,4,4); group=group(), group.add(s1,s2,s3)") {
    THEN("group.subdivide()") {
      auto s1_sphere = std::make_shared<Sphere>(1);
      auto s2_sphere = std::make_shared<Sphere>(2);
      auto s3_sphere = std::make_shared<Sphere>(3);
      s1_sphere->SetTransform(Matrix<4, 4>::Translation(-2, -2, 0));
      s2_sphere->SetTransform(Matrix<4, 4>::Translation(-2, 2, 0));
      s3_sphere->SetTransform(Matrix<4, 4>::Scaling(4, 4, 4));
      auto group_var = std::make_shared<Group>(6);
      group_var->AddChild(s1_sphere);
      group_var->AddChild(s2_sphere);
      group_var->AddChild(s3_sphere);
      group_var->Divide(1);
      const auto children = group_var->GetChildren();
      REQUIRE(children.size() == 2);
      CHECK(children[0] == s3_sphere);
      const auto subgroup = std::dynamic_pointer_cast<Group>(children[1]);
      REQUIRE(subgroup != nullptr);
      const auto nested = subgroup->GetChildren();
      REQUIRE(nested.size() == 2);
      const auto first = std::dynamic_pointer_cast<Group>(nested[0]);
      const auto second = std::dynamic_pointer_cast<Group>(nested[1]);
      REQUIRE(first != nullptr);
      REQUIRE(second != nullptr);
      CHECK(first->GetChildren() == std::vector<std::shared_ptr<Shape>>{s1_sphere});
      CHECK(second->GetChildren() == std::vector<std::shared_ptr<Shape>>{s2_sphere});
    }
  }
}

SCENARIO("14_ext: subdividing a group with too few children") {
  GIVEN(
      "s1= sphere,s1.translate(-2,0,0),s2=sphere.translate(2,1,0), s3.sphere.translate(2,-1,0), sbgrp =group(s1, "
      "s2,s3);s4=sphere;g=groupof(sbgrp, s4)") {
    THEN("g.divide(3); g[0] = sbgrp; g[1]=s4;sbgrp.count =2; sbgrp[0] =[s1];sbgrp[1]=[s2,s3]") {
      auto s1_sphere = std::make_shared<Sphere>(1);
      auto s2_sphere = std::make_shared<Sphere>(2);
      auto s3_sphere = std::make_shared<Sphere>(3);
      s1_sphere->SetTransform(Matrix<4, 4>::Translation(-2, 0, 0));
      s2_sphere->SetTransform(Matrix<4, 4>::Translation(2, 1, 0));
      s3_sphere->SetTransform(Matrix<4, 4>::Translation(2, -1, 0));
      auto sbgrp = std::make_shared<Group>(7);
      sbgrp->AddChild(s1_sphere);
      sbgrp->AddChild(s2_sphere);
      sbgrp->AddChild(s3_sphere);
      auto s4_sphere = std::make_shared<Sphere>(4);
      auto group_var = std::make_shared<Group>(8);
      group_var->AddChild(sbgrp);
      group_var->AddChild(s4_sphere);
      group_var->Divide(3);
      const auto children = group_var->GetChildren();
      REQUIRE(children.size() == 2);
      CHECK(children[0] == sbgrp);
      CHECK(children[1] == s4_sphere);
      const auto nested = sbgrp->GetChildren();
      REQUIRE(nested.size() == 2);
      const auto left = std::dynamic_pointer_cast<Group>(nested[0]);
      const auto right = std::dynamic_pointer_cast<Group>(nested[1]);
      REQUIRE(left != nullptr);
      REQUIRE(right != nullptr);
      CHECK(left->GetChildren() == std::vector<std::shared_ptr<Shape>>{s1_sphere});
      const std::vector<std::shared_ptr<Shape>> expected_right{s2_sphere, s3_sphere};
      CHECK(right->GetChildren() == expected_right);
    }
  }
}

// SCENARIO("14_ext: subdividing a csg shape subdivides its children") {
//   GIVEN(
//       "s1=sphere;s1.translate(-1.5,0,0);s2=sphere;s2.translate(1.5,0,0);left= group(s1, "
//       "s2);s3=sphere;s3.translate(0,0,-1.5);s4=sphere;s4.translate(0,0,1.5);right=group(s3,s4);csg= "
//       "csg(diff,left,right)") {
//     THEN("csg_var.divide(1);left(0) = group(s1);left(1)=group(s2);right(0)=group(s3);right(1)=group(s4)") {
//       auto s1_sphere = std::make_shared<Sphere>(1);
//       auto s2_sphere = std::make_shared<Sphere>(2);
//       s1_sphere->SetTransform(Matrix<4, 4>::Translation(-1.5, 0, 0));
//       s2_sphere->SetTransform(Matrix<4, 4>::Translation(1.5, 0, 0));
//       auto left_group = std::make_shared<Group>(9);
//       left_group->AddChild(s1_sphere);
//       left_group->AddChild(s2_sphere);
//
//       auto s3_sphere = std::make_shared<Sphere>(3);
//       auto s4_sphere = std::make_shared<Sphere>(4);
//       s3_sphere->SetTransform(Matrix<4, 4>::Translation(0, 0, -1.5));
//       s4_sphere->SetTransform(Matrix<4, 4>::Translation(0, 0, 1.5));
//       auto right_group = std::make_shared<Group>(10);
//       right_group->AddChild(s3_sphere);
//       right_group->AddChild(s4_sphere);
//
//       auto csg_var = std::make_shared<CSG>(CSGOperation::Difference, left_group, right_group);
//       csg_var->Divide(1);
//
//       CHECK(left_group->GetChildren().size() == 2);
//       CHECK(left_group->GetChildren()[0] == s1_sphere || left_group->GetChildren()[0] == s2_sphere);
//       CHECK(left_group->GetChildren()[1] == s1_sphere || left_group->GetChildren()[1] == s2_sphere);
//
//       CHECK(right_group->GetChildren().size() == 2);
//       CHECK(right_group->GetChildren()[0] == s3_sphere || right_group->GetChildren()[0] == s4_sphere);
//       CHECK(right_group->GetChildren()[1] == s3_sphere || right_group->GetChildren()[1] == s4_sphere);
//     }
//   }
// }

}  // namespace ray_tracer
