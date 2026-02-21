#include "app/shape.h"
#include "app/material.h"
#include "app/matrix.h"
#include "app/ray.h"
#include "app/intersection.h"
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>

namespace ray_tracer {
class TestShape: public Shape {
public:
explicit TestShape(const std::size_t shape_id): Shape(shape_id) {}
  ~TestShape() override = default;
  [[nodiscard]] auto LocalIntersect(const Ray& ray) const -> std::vector<Intersection> override {
    (void)ray;
    return {};
  };
};

SCENARIO("9: The default material of shape") {
  GIVEN("s = test_shape"){
    THEN("s.material is the default material") {
      TestShape const shape1(1);
      Material default_material;
      CHECK(shape1.GetMaterial() == default_material);
    }
  }
}

SCENARIO("9: The Default Transformation") {
  GIVEN("s = test_shape"){
    THEN("s.transform is the identity matrix") {
      TestShape const shape1(1);
      CHECK(shape1.GetTransform() == ray_tracer::Matrix<4, 4>::IdentityMatrix());
    }
  }
}

SCENARIO("9: Assigning a transformation") {
  GIVEN("s = test_shape, t = translation(2,3,4)"){
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
}

