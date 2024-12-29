#include <app/intersection.h>            // for Intersection
#include <app/matrix.h>                  // for matrix
#include <app/point.h>                   // for tuple
#include <app/ray.h>                     // for Ray
#include <app/sphere.h>                  // for Sphere
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {

SCENARIO("Creating and Querying a Ray") {
  GIVEN("origin = point(1,2,3), direction = vector(4,5,6)") {
    THEN("Ray(origin, direction) = Ray(point(1,2,3), vector(4,5,6))") {
      ray_tracer::Point origin = ray_tracer::Point(1, 2, 3);
      ray_tracer::Vec3 direction = ray_tracer::Vec3(4, 5, 6);
      ray_tracer::Ray ray(origin, direction);
      CHECK(ray.GetOrigin() == origin);
      CHECK(ray.GetDirection() == direction);
    }
  }
}
SCENARIO("Computing a Point from a Distance") {
  GIVEN("r = Ray(point(2,3,4), vector(1,0,0))") {
    ray_tracer::Ray ray(ray_tracer::Point(2, 3, 4), ray_tracer::Vec3(1, 0, 0));
    THEN("position(r, 0) = point(2,3,4)") { CHECK(ray.Position(0) == ray_tracer::Point(2, 3, 4)); }
    THEN("position(r, 1) = point(3,3,4)") { CHECK(ray.Position(1) == ray_tracer::Point(3, 3, 4)); }
    THEN("position(r, -1) = point(1,3,4)") { CHECK(ray.Position(-1) == ray_tracer::Point(1, 3, 4)); }
    THEN("position(r, 2.5) = point(4.5,3,4)") { CHECK(ray.Position(2.5) == ray_tracer::Point(4.5, 3, 4)); }
  }
}
SCENARIO("A Ray intersects a sphere at two points") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = Intersect(ray, s);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = 4.0") { CHECK(xs[0].GetTime() == 4.0); }
    THEN("xs[1] = 6.0") { CHECK(xs[1].GetTime() == 6.0); }
  }
}
SCENARIO("A Ray intersects a sphere at a tangent") {
  GIVEN("r = Ray(point(0,1,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 1, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = Intersect(ray, s);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = 5.0") { CHECK(xs[0].GetTime() == 5.0); }
    THEN("xs[1] = 5.0") { CHECK(xs[1].GetTime() == 5.0); }
  }
}

SCENARIO("A Ray misses a sphere") {
  GIVEN("r = Ray(point(0,2,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 2, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = Intersect(ray, s);
    THEN("xs.count = 0") { CHECK(xs.empty()); }
  }
}

SCENARIO("A Ray originates inside a sphere") {
  GIVEN("r = Ray(point(0,0,0), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, 0), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = Intersect(ray, s);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = -1.0") { CHECK(xs[0].GetTime() == -1.0); }
    THEN("xs[1] = 1.0") { CHECK(xs[1].GetTime() == 1.0); }
  }
}

SCENARIO("A sphere is behind a ray") {
  GIVEN("r = Ray(point(0,0,5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, 5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = Intersect(ray, s);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0] = -6.0") { CHECK(xs[0].GetTime() == -6.0); }
    THEN("xs[1] = -4.0") { CHECK(xs[1].GetTime() == -4.0); }
  }
}

SCENARIO("An intersection encapsulates t and object") {
  GIVEN("s=Sphere(),i=Intersection(3.5,s)") {
    ray_tracer::Sphere sphe;
    ray_tracer::Intersection intersectn(3.5, sphe);
    THEN("i.t = 3.5") { CHECK(intersectn.GetTime() == 3.5); }
    THEN("i.object = s") { CHECK(intersectn.GetObject() == sphe); }
  }
}

SCENARIO("Aggregating Intersections") {
  GIVEN("s=Sphere(),i1=Intersection(1,s),i2=Intersection(2,s)") {
    ray_tracer::Sphere sphe;
    ray_tracer::Intersection intersectn1(1, sphe);
    ray_tracer::Intersection intersectn2(2, sphe);
    THEN("xs = Intersection(1,s),Intersection(2,s)") {
      std::vector<ray_tracer::Intersection> xs{intersectn1, intersectn2};
      THEN("xs.count = 2") { CHECK(xs.size() == 2); }
      THEN("xs[0].t = 1") { CHECK(xs[0].GetTime() == 1); }
      THEN("xs[1].t = 2") { CHECK(xs[1].GetTime() == 2); }
    }
  }
}
SCENARIO("Intersect sets the object on the intersection") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere s;
    auto xs = Intersect(ray, s);
    THEN("xs.count = 2") { CHECK(xs.size() == 2); }
    THEN("xs[0].object = s") { CHECK(xs[0].GetObject() == s); }
    THEN("xs[1].object = s") { CHECK(xs[1].GetObject() == s); }
  }
}

SCENARIO("The hit, when all intersections have positive t") {
  GIVEN("s=Sphere(),i1=Intersection(1,s),i2=Intersection(2,s)") {
    ray_tracer::Sphere sphe;
    ray_tracer::Intersection intersectn1(1, sphe);
    ray_tracer::Intersection intersectn2(2, sphe);
    THEN("xs = Intersection(1,s),Intersection(2,s)") {
      std::vector<ray_tracer::Intersection> xs{intersectn1, intersectn2};
      THEN("i = hit(xs)") {
        auto i = Hit(xs);
        THEN("i = i1") { CHECK(i == intersectn1); }
      }
    }
  }
}

SCENARIO("The hit, when some intersections have negative t") {
  GIVEN("s=Sphere(),i1=Intersection(-1,s),i2=Intersection(1,s)") {
    ray_tracer::Sphere sphe;
    ray_tracer::Intersection intersectn1(-1, sphe);
    ray_tracer::Intersection intersectn2(1, sphe);
    THEN("xs = Intersection(-1,s),Intersection(1,s)") {
      std::vector<ray_tracer::Intersection> xs{intersectn1, intersectn2};
      THEN("i = hit(xs)") {
        auto i = Hit(xs);
        THEN("i = i2") { CHECK(i == intersectn2); }
      }
    }
  }
}
SCENARIO("The hit, when all intersections have negative t") {
  GIVEN("s=Sphere(),i1=Intersection(-2,s),i2=Intersection(-1,s)") {
    ray_tracer::Sphere sphe;
    ray_tracer::Intersection intersectn1(-2, sphe);
    ray_tracer::Intersection intersectn2(-1, sphe);
    THEN("xs = Intersection(-2,s),Intersection(-1,s)") {
      std::vector<ray_tracer::Intersection> xs{intersectn1, intersectn2};
      THEN("i = hit(xs)") {
        auto i = Hit(xs);
        THEN("i = nullptr") { CHECK(i == std::nullopt); }
      }
    }
  }
}

SCENARIO("The hit is always the lowest nonnegative intersection") {
  GIVEN("s=Sphere(),i1=Intersection(5,s),i2=Intersection(7,s),i3=Intersection(-3,s),i4=Intersection(2,s)") {
    ray_tracer::Sphere sphe;
    ray_tracer::Intersection intersectn1(5, sphe);
    ray_tracer::Intersection intersectn2(7, sphe);
    ray_tracer::Intersection intersectn3(-3, sphe);
    ray_tracer::Intersection intersectn4(2, sphe);
    THEN("xs = Intersection(5,s),Intersection(7,s),Intersection(-3,s),Intersection(2,s)") {
      std::vector<ray_tracer::Intersection> xs{intersectn1, intersectn2, intersectn3, intersectn4};
      THEN("i = hit(xs)") {
        auto i = Hit(xs);
        THEN("i = i4") { CHECK(i == intersectn4); }
      }
    }
  }
}

}  // namespace ray_tracer
