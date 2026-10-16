#include <catch2/catch_test_macros.hpp>

#include "app/shapes/cube.h"
#include "app/primitives/point.h"
#include "app/rendering/ray.h"
#include "app/primitives/vec3.h"
namespace ray_tracer {

SCENARIO("12: A ray intersects a cube") {
  //| +x point(5, 0.5, 0) | vector(-1, 0, 0) | 4 | 6 |
  // -x point(-5, 0.5, 0) | vector(1, 0, 0) | 4 | 6 |
  // +y | point(0.5, 5, 0) | vector(0, -1, 0) | 4 | 6 |
  // -y point(0.5, -5, 0) | vector(0, 1, 0) | 4 | 6 |
  // +z | point(0.5, 0, 5) | vector(0, 0, -1) | 4 | 6 |
  // -z point(0.5, 0, -5) | vector(0, 0, 1) | 4 | 6 |
  // | inside | point(0, 0.5, 0) | vector(0, 0, 1) | -1 | 1 |
  GIVEN("c =cube(), r =ray(point(5, 0.5, 0), vec3(-1, 0, 0))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = 4, xs.t2 =6") {
      auto cube = Cube(1);
      auto ray = Ray(Point(5, 0.5, 0), Vec3(-1, 0, 0));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == 4);
      CHECK(intersections[1].GetTime() == 6);
    }
  }
  GIVEN("c =cube(), r =ray(point(-5, 0.5, 0), vec3(1, 0, 0))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = 4, xs.t2 =6") {
      auto cube = Cube(1);
      auto ray = Ray(Point(-5, 0.5, 0), Vec3(1, 0, 0));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == 4);
      CHECK(intersections[1].GetTime() == 6);
    }
  }
  GIVEN("c =cube(), r =ray(point(0.5, 5, 0), vec3(0, -1, 0))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = 4, xs.t2 =6") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0.5, 5, 0), Vec3(0, -1, 0));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == 4);
      CHECK(intersections[1].GetTime() == 6);
    }
  }
  GIVEN("c =cube(), r =ray(point(0.5, -5, 0), vec3(0, 1, 0))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = 4, xs.t2 =6") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0.5, -5, 0), Vec3(0, 1, 0));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == 4);
      CHECK(intersections[1].GetTime() == 6);
    }
  }
  GIVEN("c =cube(), r =ray(point(0.5, 0, 5), vec3(0, 0, -1))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = 4, xs.t2 =6") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0.5, 0, 5), Vec3(0, 0, -1));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == 4);
      CHECK(intersections[1].GetTime() == 6);
    }
  }
  GIVEN("c =cube(), r =ray(point(0.5, 0, -5), vec3(0, 0, 1))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = 4, xs.t2 =6") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0.5, 0, -5), Vec3(0, 0, 1));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == 4);
      CHECK(intersections[1].GetTime() == 6);
    }
  }
  GIVEN("c =cube(), r =ray(point(0, 0.5, 0), vec3(0, 0, 1))") {
    THEN("xs = c.local_intersect(ray) = xs.t1 = -1, xs.t2 =1") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0, 0.5, 0), Vec3(0, 0, 1));
      auto intersections = cube.Intersect(ray);
      REQUIRE(intersections.size() == 2);
      CHECK(intersections[0].GetTime() == -1);
      CHECK(intersections[1].GetTime() == 1);
    }
  }
}
SCENARIO("12: A ray misses a cube") {
  GIVEN("c =cube(), r =ray(point(-2, 0, 0), vec3(0.2673, 0.5345, 0.8018))") {
    THEN("xs = c.local_intersect(ray) is empty") {
      auto cube = Cube(1);
      auto ray = Ray(Point(-2, 0, 0), Vec3(0.2673, 0.5345, 0.8018));
      auto intersections = cube.Intersect(ray);
      CHECK(intersections.empty());
    }
  }
  GIVEN("c =cube(), r =ray(point(0, -2, 0), vec3(0.8018, 0.2673, 0.5345))") {
    THEN("xs = c.local_intersect(ray) is empty") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0, -2, 0), Vec3(0.8018, 0.2673, 0.5345));
      auto intersections = cube.Intersect(ray);
      CHECK(intersections.empty());
    }
  }
  GIVEN("c =cube(), r =ray(point(0, 0, -2), vec3(0.5345, 0.8018, 0.2673))") {
    THEN("xs = c.local_intersect(ray) is empty") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0, 0, -2), Vec3(0.5345, 0.8018, 0.2673));
      auto intersections = cube.Intersect(ray);
      CHECK(intersections.empty());
    }
  }
  GIVEN("c=cube(), r=ray(point(2, 0, 2), vec3(0, 0, -1)") {
    THEN("xs = c.local_intersect(ray) is empty") {
      auto cube = Cube(1);
      auto ray = Ray(Point(2, 0, 2), Vec3(0, 0, -1));
      auto intersections = cube.Intersect(ray);
      CHECK(intersections.empty());
    }
  }
  GIVEN("c=cube(), r=ray(point(0, 2, 2), vec3(0, -1, 0)") {
    THEN("xs = c.local_intersect(ray) is empty") {
      auto cube = Cube(1);
      auto ray = Ray(Point(0, 2, 2), Vec3(0, -1, 0));
      auto intersections = cube.Intersect(ray);
      CHECK(intersections.empty());
    }
  }
  GIVEN("c=cube(), r=ray(point(2, 2, 0), vec3(-1, 0, 0)") {
    THEN("xs = c.local_intersect(ray) is empty") {
      auto cube = Cube(1);
      auto ray = Ray(Point(2, 2, 0), Vec3(-1, 0, 0));
      auto intersections = cube.Intersect(ray);
      CHECK(intersections.empty());
    }
  }
}

SCENARIO("12: The normal of the surface of the cube") {
  GIVEN("c =cube(), n =c.local_normal_at(point(1, 0.5, -0.8))") {
    THEN("n =vec3(1, 0, 0)") {
      auto cube = Cube(1);
      auto normal = cube.NormalAt(Point(1, 0.5, -0.8));
      CHECK(normal == Vec3(1, 0, 0));
    }
  }
  GIVEN("c =cube(), n =c.local_normal_at(point(-1, -0.2, 0.9))") {
    THEN("n =vec3(-1, 0, 0)") {
      auto cube = Cube(1);
      auto normal = cube.NormalAt(Point(-1, -0.2, 0.9));
      CHECK(normal == Vec3(-1, 0, 0));
    }
  }
  GIVEN("c =cube(), n =c.local_normal_at(point(-0.4, 1, -0.1))") {
    THEN("n =vec3(0, 1, 0)") {
      auto cube = Cube(1);
      auto normal = cube.NormalAt(Point(-0.4, 1, -0.1));
      CHECK(normal == Vec3(0, 1, 0));
    }
  }
  GIVEN("c =cube(), n =c.local_normal_at(point(-0.6, 0.3, 1))") {
    THEN("n =vec3(0, 0, 1)") {
      auto cube = Cube(1);
      auto normal = cube.NormalAt(Point(-0.6, 0.3, 1));
      CHECK(normal == Vec3(0, 0, 1));
    }
  }
  GIVEN("c=cube(), n=c.local_normal_at(point(0.4, 0.4,-1))") {
    THEN("n =vec3(0, 0, -1)") {
      auto cube = Cube(1);
      auto normal = cube.NormalAt(Point(0.4, 0.4, -1));
      CHECK(normal == Vec3(0, 0, -1));
    }
  }
  GIVEN("c=cube(), n=c.local_normal_at(point(1, 1, 0))") {
    THEN("n =vec3(1, 0, 0)") {
      auto cube = Cube(1);
      auto normal = cube.NormalAt(Point(1, 1, 0));
      CHECK(normal == Vec3(1, 0, 0));
    }
  }
  GIVEN("c=cube(), n=c.local_normal_at(point(-1, -1, -1))") {
   THEN("n =vec3(-1, 0, 0)") {
     auto cube = Cube(1);
     auto normal = cube.NormalAt(Point(-1, -1, -1));
     CHECK(normal == Vec3(-1, 0, 0));
   }}
}

}  // namespace ray_tracer
