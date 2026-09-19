#include <catch2/catch_test_macros.hpp>
#include <limits>

#include <cmath>
#include <cstddef>
#include <memory>
#include <numbers>
#include <vector>
#include "app/bounding_box.h"
#include "app/cone.h"
#include "app/cube.h"
#include "app/cylinder.h"
#include "app/group.h"
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
    (void)ray;
    return {};
  };
  [[nodiscard]] auto LocalNormalAt(const ray_tracer::Point& point) const -> ray_tracer::Vec3 override {
    return {point.GetX(), point.GetY(), point.GetZ()};
  };
  auto BoundsOf() const -> BoundingBox override { return {Point(-1, -1, -1), Point(1, 1, 1)}; }
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

SCENARIO("Checking to see if a box contains a given box") {
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

}  // namespace ray_tracer