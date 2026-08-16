#include <catch2/catch_test_macros.hpp>

// #include "app/cylinder.h"
#include <limits>
#include "app/cylinder.h"
#include "app/point.h"
#include "app/ray.h"
#include "app/utilities.h"
#include "app/vec3.h"

namespace ray_tracer {
SCENARIO("13: A ray misses a cylinder") {
  GIVEN("cyl = cylinder(), dir = normalize(<1, 0, 0>) , ray = ray(<1, 0,0>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 1, 0).Normalize();
      auto ray = Ray(Point(1, 0, 0), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<0, 1, 0>) , ray = ray(<0, 0, 0>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 1, 0).Normalize();
      auto ray = Ray(Point(0, 0, 0), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<1, 1, 1>) , ray = ray(<0, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 1, 0).Normalize();
      auto ray = Ray(Point(0, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
}
SCENARIO("13: A ray intersects a cylinder") {
  GIVEN("cyl = cylinder(), dir = normalize(<0, 0, 1>) , ray = ray(<1, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(2);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(1, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
      CHECK(intersections_xs[0].GetTime() == 5);
      CHECK(intersections_xs[1].GetTime() == 5);
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<0, 0, 1>) , ray = ray(<0, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
      CHECK(intersections_xs[0].GetTime() == 4);
      CHECK(intersections_xs[1].GetTime() == 6);
    }
  }
  GIVEN("cyl = cylinder(), dir = normalize(<0.1, 1, 1>) , ray = ray(<0.5, 0, -5>, dir),  ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      auto dir = Vec3(0.1, 1, 1).Normalize();
      auto ray = Ray(Point(0.5, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
      CHECK(utilities::Equal(6.80798, intersections_xs[0].GetTime()));
      CHECK(utilities::Equal(7.08872, intersections_xs[1].GetTime()));
    }
  }
}

SCENARIO("13: Normal vector on a cylinder") {
  GIVEN("cyl = cylinder(),normal = local_normal_at(cyl, <point>") {
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(1, 0, 0));
      CHECK(normal == Vec3(1, 0, 0));
    }
  }
  GIVEN("cyl = cylinder(),normal = local_normal_at(cyl, <point>") {
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(0, 5, -1));
      CHECK(normal == Vec3(0, 0, -1));
    }
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(0, -2, 1));
      CHECK(normal == Vec3(0, 0, 1));
    }
    THEN(" n = <normal>") {
      auto cyl = Cylinder(1);
      auto normal = cyl.NormalAt(Point(-1, 1, 0));
      CHECK(normal == Vec3(-1, 0, 0));
    }
  }
}
SCENARIO("13: The default min and max for a cylinder") {
  GIVEN("cyl = cylinder()") {
    THEN("cyl.minimum == -infinity") {
      auto cyl = Cylinder(1);
      CHECK(cyl.GetMinimum() == -std::numeric_limits<double>::infinity());
    }
    THEN("cyl.maximum == infinity") {
      auto cyl = Cylinder(1);
      CHECK(cyl.GetMaximum() == std::numeric_limits<double>::infinity());
    }
  }
}

SCENARIO("13: Intersecting a constrained cylinder") {
  GIVEN(
      "cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, dir = normalize(<0.1, 1, 0>) , ray = ray(<0, 1.5, 0>, dir), "
      " ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      auto dir = Vec3(0.1, 1, 1).Normalize();
      auto ray = Ray(Point(0, 1.5, 0), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN(
      "cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, dir = normalize(<0, 0, 1>) , ray = ray(<0, 3, -5>, dir), "
      " ") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 3, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, dir = <0,0,1> , ray =(<0,0,-5>,dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 0, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, dir = <0,0,1> , ray =(<0,2,-5>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 0") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 2, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, dir = <0,0,1> , ray =(<0,1,-5>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 1, -5), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.empty());
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, dir = <0,0,1> , ray =(<0,1.5,-2>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      auto dir = Vec3(0, 0, 1).Normalize();
      auto ray = Ray(Point(0, 1.5, -2), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
    }
  }
}
SCENARIO("13: the default closed val for a cylinder") {
  GIVEN("cyl = cylinder()") {
    THEN("cyl.closed == false") {
      auto cyl = Cylinder(1);
      CHECK(cyl.IsClosed() == false);
    }
  }
}

SCENARIO("13: Intersecting the caps of a closed cylinder") {
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, cyl.closed = true, dir = <0, -1, 0> , ray = (<0, 3, 0>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto dir = Vec3(0, -1, 0).Normalize();
      auto ray = Ray(Point(0, 3, 0), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, cyl.closed = true, dir = <0, -1, 2> , ray = (<0, 3, -2>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto dir = Vec3(0, -1, 2).Normalize();
      auto ray = Ray(Point(0, 3, -2), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2), cyl.closed = true, dir = <0, -1, 1> , ray = (<0, 4, -2>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto dir = Vec3(0, -1, 1).Normalize();
      auto ray = Ray(Point(0, 4, -2), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, cyl.closed = true, dir = <0, 1, 2> , ray = (<0, 0, -2>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto dir = Vec3(0, 1, 2).Normalize();
      auto ray = Ray(Point(0, 0, -2), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
      CHECK(intersections_xs.size() == 2);
    }
  }
  GIVEN("cyl = cylinder(), cyl.minimum = 1, cyl.maximum = 2, cyl.closed = true, dir = <0, 1, 1> , ray = (<0, -1, -2>, dir)") {
    THEN("intersections_xs = cyl.local_intersect(ray).count == 2") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto dir = Vec3(0, 1, 1).Normalize();
      auto ray = Ray(Point(0, -1, -2), dir);
      auto intersections_xs = cyl.LocalIntersect(ray);
    }
  }
}

SCENARIO("The normal vector on a cylinder's end caps") {
  GIVEN("cyl =cylinder(1), min=1, max=2, closed= true,n=local_normal_at(<point>)") {
    THEN("n = <normal>") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto normal_vec = cyl.NormalAt(Point(0, 1, 0));
      CHECK(normal_vec == Vec3(0, -1, 0));
    }
  }
  GIVEN("point = point((0.5, 1,0)") {
    THEN("n = (0, -1, 0)") {
      auto cyl = Cylinder(1);
      cyl.SetMinimum(1);
      cyl.SetMaximum(2);
      cyl.SetClosed(true);
      auto normal_vec = cyl.NormalAt(Point(0.5, 1, 0));
      CHECK(normal_vec == Vec3(0, -1, 0));
    }
  }
    GIVEN("point = point((0, 1, 0.5)") {
      THEN("n = (0, -1, 0)") {
        auto cyl = Cylinder(1);
        cyl.SetMinimum(1);
        cyl.SetMaximum(2);
        cyl.SetClosed(true);
        auto normal_vec = cyl.NormalAt(Point(0, 1, 0.5));
        CHECK(normal_vec == Vec3(0, -1, 0));
      }
    }
    GIVEN("point = point((0, 2, 0)") {
      THEN("n = (0, 1, 0)") {
        auto cyl = Cylinder(1);
        cyl.SetMinimum(1);
        cyl.SetMaximum(2);
        cyl.SetClosed(true);
        auto normal_vec = cyl.NormalAt(Point(0, 2, 0));
        CHECK(normal_vec == Vec3(0, 1, 0));
      }
    }
    GIVEN("point = point((0.5, 2, 0)") {
      THEN("n = (0, 1, 0)") {
        auto cyl = Cylinder(1);
        cyl.SetMinimum(1);
        cyl.SetMaximum(2);
        cyl.SetClosed(true);
        auto normal_vec = cyl.NormalAt(Point(0.5, 2, 0));
        CHECK(normal_vec == Vec3(0, 1, 0));
      }
    }
    GIVEN("point = point((0, 2, 0.5)") {
      THEN("n = (0, 1, 0)") {
        auto cyl = Cylinder(1);
        cyl.SetMinimum(1);
        cyl.SetMaximum(2);
        cyl.SetClosed(true);
        auto normal_vec = cyl.NormalAt(Point(0, 2, 0.5));
        CHECK(normal_vec == Vec3(0, 1, 0));
      }
    }
  }
}  // namespace ray_tracer