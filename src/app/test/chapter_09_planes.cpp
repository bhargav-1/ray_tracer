#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <numbers>
#include <optional>
#include <vector>
#include "app/materials/material.h"
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/vec3.h"
#include "app/rendering/bounding_box.h"
#include "app/rendering/intersection.h"
#include "app/rendering/ray.h"
#include "app/shapes/plane.h"
#include "app/shapes/shape.h"

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

SCENARIO("9: The default material of shape") {
  GIVEN("s = test_shape") {
    THEN("s.material is the default material") {
      TestShape const shape1(1);
      Material default_material;
      CHECK(shape1.GetMaterial() == default_material);
    }
  }
}

SCENARIO("9: The Default Transformation") {
  GIVEN("s = test_shape") {
    THEN("s.transform is the identity matrix") {
      TestShape const shape1(1);
      CHECK(shape1.GetTransform() == ray_tracer::Matrix<4, 4>::IdentityMatrix());
    }
  }
}

SCENARIO("9: Assigning a transformation") {
  GIVEN("s = test_shape, t = translation(2,3,4)") {
    THEN("s.transform = t") {
      TestShape shape1(1);
      constexpr ray_tracer::Matrix<4, 4> kMat = ray_tracer::Matrix<4, 4>::Translation(2, 3, 4);
      shape1.SetTransform(kMat);
      CHECK(shape1.GetTransform() == kMat);
    }
  }
}
SCENARIO("9: Assigning a material") {
  GIVEN("s = test_shape, m = test_material") {
    THEN("s.material = m") {
      TestShape shape1(1);
      Material material1;
      shape1.SetMaterial(material1);
      CHECK(shape1.GetMaterial() == material1);
    }
  }
}

SCENARIO("9: Intersecting a scaled shape with a ray") {
  GIVEN("r =ray(point(0,0,-5), vector(0,0,1)), s = test_shape with scaling(2,2,2)") {
    THEN("s.saved_ray = ray(point(0,0,-2.5), vector(0,0,0.5))") {
      TestShape shape1(1);
      constexpr ray_tracer::Matrix<4, 4> kMat = ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2);
      shape1.SetTransform(kMat);
      constexpr auto kRay = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
      auto const intersections = shape1.Intersect(kRay);
      auto const saved_ray = shape1.GetSavedRay();
      REQUIRE(saved_ray.has_value());
      if (saved_ray.has_value()) {
        CHECK(saved_ray->GetOrigin() == ray_tracer::Point(0, 0, -2.5));
        CHECK(saved_ray->GetDirection() == ray_tracer::Vec3(0, 0, 0.5));
      } else {
        FAIL("saved_ray is not set");
      }
    }
  }
}

SCENARIO("9: Intersecting a translated shape with a ray") {
  GIVEN("r = ray(point(0,0,-5), vector(0,0,1)), s = test_shape with translation(5,0,0)") {
    THEN("saved_ray = ray(point(5,0,-5), vector(0,0,1))") {
      TestShape shape1(1);
      constexpr ray_tracer::Matrix<4, 4> kMat = ray_tracer::Matrix<4, 4>::Translation(5, 0, 0);
      shape1.SetTransform(kMat);
      constexpr auto kRay = ray_tracer::Ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
      auto const intersections = shape1.Intersect(kRay);
      auto const saved_ray = shape1.GetSavedRay();
      REQUIRE(saved_ray.has_value());
      if (saved_ray.has_value()) {
        CHECK(saved_ray->GetOrigin() == ray_tracer::Point(-5, 0, -5));
        CHECK(saved_ray->GetDirection() == ray_tracer::Vec3(0, 0, 1));
      }
    }
  }
}

SCENARIO("9: Computing the normal on a translated shape") {
  GIVEN("s= test_shape(), s.Transform(0,1,0), n = normal_at(s, point(0,1.70711,-0.70711))") {
    THEN("n = vector(0,0.70711,-0.70711)") {
      TestShape shape1(1);
      shape1.SetTransform(ray_tracer::Matrix<4, 4>::Translation(0, 1, 0));
      CHECK(shape1.NormalAt(ray_tracer::Point(0, 1.70711, -0.70711)) == ray_tracer::Vec3(0, 0.70711, -0.70711));
    }
  }
}
SCENARIO("9: Computing the normal on a transformed shape") {
  GIVEN(
      "s = TestShape(), m= scaling(1, 0.5, 1)*rotation_Z(pi/5),s.SetTransform(m), n=normal_at(s, Point(0, 1/sqrt(2), "
      "-1/sqrt(2)") {
    THEN("n = vec3(0, 0.97014,-0.24254") {
      TestShape shape1(1);
      shape1.SetTransform(ray_tracer::Matrix<4, 4>::Scaling(1, 0.5, 1) *
                          ray_tracer::Matrix<4, 4>::RotationZ(std::numbers::pi / 5));
      CHECK(shape1.NormalAt(ray_tracer::Point(0, std::numbers::sqrt2 / 2, -std::numbers::sqrt2 / 2)) ==
            ray_tracer::Vec3(0, 0.97014, -0.24254));
    }
  }
}
SCENARIO("9: the normal of a plane is constant everywhere") {
  GIVEN("p = plane()") {
    WHEN(
        "n1 = local_normal_at(p, point(0,0,0), n2 = local_normal_at(p, point(10, 0, -10), n3 = local_normal_at(p, "
        "point(-5, 0, 150)") {
      THEN("n1 = n2 = n3 = vector(0,1,0)") {
        const ray_tracer::Plane plane_shape(1);
        CHECK(plane_shape.NormalAt(ray_tracer::Point(0, 0, 0)) == ray_tracer::Vec3(0, 1, 0));
        CHECK(plane_shape.NormalAt(ray_tracer::Point(10, 0, -10)) == ray_tracer::Vec3(0, 1, 0));
        CHECK(plane_shape.NormalAt(ray_tracer::Point(-5, 0, 150)) == ray_tracer::Vec3(0, 1, 0));
      }
    }
  }
}

SCENARIO("9:Intersect with a ray parallel to the plane") {
  GIVEN("p = plane(), r = ray(point(0,10,0), vector(0,0,1))") {
    THEN("there is no intersection") {
      const ray_tracer::Plane plane_shape(1);
      constexpr ray_tracer::Ray kRayShape(ray_tracer::Point(0, 10, 0), ray_tracer::Vec3(0, 0, 1));
      CHECK(plane_shape.LocalIntersect(kRayShape).empty());
    }
  }
}
SCENARIO("9: Intersect with a coplanar ray") {
  GIVEN("p = plane(), r = ray(point(0,0,0), vector(0,0,1))") {
    THEN("local_intersect(p,r).empy()==true") {
      const ray_tracer::Plane plane_shape(1);
      constexpr ray_tracer::Ray kRayShape(ray_tracer::Point(0, 0, 0), ray_tracer::Vec3(0, 0, 1));
      CHECK(plane_shape.LocalIntersect(kRayShape).empty());
    }
  }
}

SCENARIO("9: A ray intersecting a plane from above") {
  GIVEN("p = plane(), r = ray(point(0,1,0), vector(0,-1,0))") {
    THEN("local_intersect(p,r) is not empty") {
      const ray_tracer::Plane plane_shape(1);
      constexpr ray_tracer::Ray kRayShape(ray_tracer::Point(0, 1, 0), ray_tracer::Vec3(0, -1, 0));
      auto const intersections = plane_shape.LocalIntersect(kRayShape);
      REQUIRE(intersections.size() == 1);
      CHECK(intersections[0].GetTime() == 1);
      CHECK(*intersections[0].GetObject() == plane_shape);
    }
  }
}
SCENARIO("9: A ray intersecting a plane from below") {
  GIVEN("p = plane(), r = ray(point(0,-1,0), vector(0,1,0))") {
    THEN("local_intersect(p,r) is not empty") {
      const ray_tracer::Plane plane_shape(1);
      constexpr ray_tracer::Ray kRayShape(ray_tracer::Point(0, -1, 0), ray_tracer::Vec3(0, 1, 0));
      auto const intersections = plane_shape.LocalIntersect(kRayShape);
      REQUIRE(intersections.size() == 1);
      CHECK(intersections[0].GetTime() == 1);
      CHECK(*intersections[0].GetObject() == plane_shape);
    }
  }
}

}  // namespace ray_tracer
