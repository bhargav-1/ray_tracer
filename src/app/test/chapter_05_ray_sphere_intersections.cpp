#include <app/rendering/intersection.h>            // for Intersection
#include <app/rendering/ray_tracing_engine.h>     // for Intersect, Hit
#include <app/primitives/matrix.h>                  // for matrix
#include <app/primitives/point.h>                   // for tuple
#include <app/rendering/ray.h>                     // for Ray
#include <app/shapes/sphere.h>                  // for Sphere
#include <app/primitives/tuple.h>                   // for tuple
#include <app/primitives/vec3.h>                    // for vector
#include <vector>
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section

namespace ray_tracer {

SCENARIO("Creating and Querying a Ray") {
  GIVEN("origin = point(1,2,3), direction = vector(4,5,6)") {
    THEN("Ray(origin, direction) = Ray(point(1,2,3), vector(4,5,6))") {
      const ray_tracer::Point origin = ray_tracer::Point(1, 2, 3);
      const ray_tracer::Vec3 direction = ray_tracer::Vec3(4, 5, 6);
      const ray_tracer::Ray ray(origin, direction);
      CHECK(ray.GetOrigin() == origin);
      CHECK(ray.GetDirection() == direction);
    }
  }
}
SCENARIO("Computing a Point from a Distance") {
  GIVEN("r = Ray(point(2,3,4), vector(1,0,0))") {
    const ray_tracer::Ray ray(ray_tracer::Point(2, 3, 4), ray_tracer::Vec3(1, 0, 0));
    THEN("position(r, 0) = point(2,3,4)") { CHECK(ray.Position(0) == ray_tracer::Point(2, 3, 4)); }
    THEN("position(r, 1) = point(3,3,4)") { CHECK(ray.Position(1) == ray_tracer::Point(3, 3, 4)); }
    THEN("position(r, -1) = point(1,3,4)") { CHECK(ray.Position(-1) == ray_tracer::Point(1, 3, 4)); }
    THEN("position(r, 2.5) = point(4.5,3,4)") { CHECK(ray.Position(2.5) == ray_tracer::Point(4.5, 3, 4)); }
  }
}
SCENARIO("A Ray intersects a sphere at two points") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    const ray_tracer::Sphere s_spher(1);
    auto xs_intersect = s_spher.LocalIntersect(ray);
    THEN("xs.count = 2") { CHECK(xs_intersect.size() == 2); }
    THEN("xs[0] = 4.0") { CHECK(xs_intersect[0].GetTime() == 4.0); }
    THEN("xs[1] = 6.0") { CHECK(xs_intersect[1].GetTime() == 6.0); }
  }
}
SCENARIO("A Ray intersects a sphere at a tangent") {
  GIVEN("r = Ray(point(0,1,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 1, -5), ray_tracer::Vec3(0, 0, 1));
    const ray_tracer::Sphere s_sphere(1);
    auto xs_intersect = s_sphere.LocalIntersect(ray);
    THEN("xs.count = 2") { CHECK(xs_intersect.size() == 2); }
    THEN("xs[0] = 5.0") { CHECK(xs_intersect[0].GetTime() == 5.0); }
    THEN("xs[1] = 5.0") { CHECK(xs_intersect[1].GetTime() == 5.0); }
  }
}

SCENARIO("A Ray misses a sphere") {
  GIVEN("r = Ray(point(0,2,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 2, -5), ray_tracer::Vec3(0, 0, 1));
    const ray_tracer::Sphere s_sphere(1);
    auto xs_intersect = s_sphere.LocalIntersect(ray);
    THEN("xs.count = 0") { CHECK(xs_intersect.empty()); }
  }
}

SCENARIO("A Ray originates inside a sphere") {
  GIVEN("r = Ray(point(0,0,0), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 0, 0), ray_tracer::Vec3(0, 0, 1));
    const ray_tracer::Sphere spere(1);
    auto xs_interset = spere.LocalIntersect(ray);
    THEN("xs.count = 2") { CHECK(xs_interset.size() == 2); }
    THEN("xs[0] = -1.0") { CHECK(xs_interset[0].GetTime() == -1.0); }
    THEN("xs[1] = 1.0") { CHECK(xs_interset[1].GetTime() == 1.0); }
  }
}

SCENARIO("A sphere is behind a ray") {
  GIVEN("r = Ray(point(0,0,5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 0, 5), ray_tracer::Vec3(0, 0, 1));
    const ray_tracer::Sphere sphere(1);
    auto xs_intersect = sphere.LocalIntersect(ray);
    THEN("xs.count = 2") { CHECK(xs_intersect.size() == 2); }
    THEN("xs[0] = -6.0") { CHECK(xs_intersect[0].GetTime() == -6.0); }
    THEN("xs[1] = -4.0") { CHECK(xs_intersect[1].GetTime() == -4.0); }
  }
}

SCENARIO("An intersection encapsulates t and object") {
  GIVEN("s=Sphere(),i=Intersection(3.5,s)") {
    const ray_tracer::Sphere sphe(1);
    const ray_tracer::Intersection intersectn(3.5, sphe);
    THEN("i.t = 3.5") { CHECK(intersectn.GetTime() == 3.5); }
    THEN("i.object = s") { CHECK(*intersectn.GetObject() == sphe); }
  }
}

SCENARIO("Aggregating Intersections") {
  GIVEN("s=Sphere(),i1=Intersection(1,s),i2=Intersection(2,s)") {
    const ray_tracer::Sphere sphe(1);
    const ray_tracer::Intersection intersectn1(1, sphe);
    const ray_tracer::Intersection intersectn2(2, sphe);
    THEN("xs = Intersection(1,s),Intersection(2,s)") {
      const std::vector<ray_tracer::Intersection> xs_intersection{intersectn1, intersectn2};
      THEN("xs.count = 2") { CHECK(xs_intersection.size() == 2); }
      THEN("xs[0].t = 1") { CHECK(xs_intersection[0].GetTime() == 1); }
      THEN("xs[1].t = 2") { CHECK(xs_intersection[1].GetTime() == 2); }
    }
  }
}
SCENARIO("Intersect sets the object on the intersection") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s=Sphere(),xs=intersect(r,s)") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    const ray_tracer::Sphere s_sphere(1);
    auto xs_intersect = s_sphere.LocalIntersect(ray);
    THEN("xs.count = 2") { CHECK(xs_intersect.size() == 2); }
    THEN("xs[0].object = s") { CHECK(*xs_intersect[0].GetObject() == s_sphere); }
    THEN("xs[1].object = s") { CHECK(*xs_intersect[1].GetObject() == s_sphere); }
  }
}

SCENARIO("The hit, when all intersections have positive t") {
  GIVEN("s=Sphere(),i1=Intersection(1,s),i2=Intersection(2,s)") {
    const ray_tracer::Sphere sphe(1);
    const ray_tracer::Intersection intersectn1(1, sphe);
    const ray_tracer::Intersection intersectn2(2, sphe);
    THEN("xs = Intersection(1,s),Intersection(2,s)") {
      const std::vector<ray_tracer::Intersection> xs_intersection{intersectn1, intersectn2};
      THEN("i = hit(xs)") {
        auto i = Hit(xs_intersection); // NOLINT
        THEN("i = i1") { CHECK(i == intersectn1); }
      }
    }
  }
}

SCENARIO("The hit, when some intersections have negative t") {
  GIVEN("s=Sphere(),i1=Intersection(-1,s),i2=Intersection(1,s)") {
    const ray_tracer::Sphere sphe(1);
    const ray_tracer::Intersection intersectn1(-1, sphe);
    const ray_tracer::Intersection intersectn2(1, sphe);
    THEN("xs = Intersection(-1,s),Intersection(1,s)") {
      const std::vector<ray_tracer::Intersection> xs_intersection{intersectn1, intersectn2};
      THEN("i = hit(xs)") {
        auto i = Hit(xs_intersection); // NOLINT
        THEN("i = i2") { CHECK(i == intersectn2); }
      }
    }
  }
}
SCENARIO("The hit, when all intersections have negative t") {
  GIVEN("s=Sphere(),i1=Intersection(-2,s),i2=Intersection(-1,s)") {
    const ray_tracer::Sphere sphe(1);
    const ray_tracer::Intersection intersectn1(-2, sphe);
    const ray_tracer::Intersection intersectn2(-1, sphe);
    THEN("xs = Intersection(-2,s),Intersection(-1,s)") {
      const std::vector<ray_tracer::Intersection> xs_intersection{intersectn1, intersectn2};
      THEN("i = hit(xs)") {
        auto i = Hit(xs_intersection); // NOLINT
        THEN("i = nullptr") { CHECK(i == std::nullopt); } // NOLINT
      }
    }
  }
}

SCENARIO("The hit is always the lowest nonnegative intersection") {
  GIVEN("s=Sphere(),i1=Intersection(5,s),i2=Intersection(7,s),i3=Intersection(-3,s),i4=Intersection(2,s)") {
    const ray_tracer::Sphere sphe(1);
    const ray_tracer::Intersection intersectn1(5, sphe);
    const ray_tracer::Intersection intersectn2(7, sphe);
    const ray_tracer::Intersection intersectn3(-3, sphe);
    const ray_tracer::Intersection intersectn4(2, sphe);
    THEN("xs = Intersection(5,s),Intersection(7,s),Intersection(-3,s),Intersection(2,s)") {
      std::vector<ray_tracer::Intersection> xs_inte{intersectn1, intersectn2, intersectn3, intersectn4}; // NOLINT
      THEN("i = hit(xs)") {
        auto i = Hit(xs_inte); // NOLINT
        THEN("i = i4") { CHECK(i == intersectn4); }
      }
    }
  }
}

SCENARIO("Translating a ray") {
  GIVEN("r = Ray(point(1,2,3), vector(0,1,0)),m = translation(3,4,5),r2 = transform(r,m)") {
    const ray_tracer::Ray ray(ray_tracer::Point(1, 2, 3), ray_tracer::Vec3(0, 1, 0));
    const ray_tracer::Matrix<4, 4> mat = ray_tracer::Matrix<4, 4>::Translation(3, 4, 5);
    const ray_tracer::Ray ray2 = ray.Transform(mat);
    THEN("r2.origin = point(4,6,8)") { CHECK(ray2.GetOrigin() == ray_tracer::Point(4, 6, 8)); }
    THEN("r2.direction = vector(0,1,0)") { CHECK(ray2.GetDirection() == ray_tracer::Vec3(0, 1, 0)); }
  }
}

SCENARIO("Scaling a ray") {
  GIVEN("r = Ray(point(1,2,3), vector(0,1,0)),m = scaling(2,3,4),r2 = transform(r,m)") {
    const ray_tracer::Ray ray(ray_tracer::Point(1, 2, 3), ray_tracer::Vec3(0, 1, 0));
    const ray_tracer::Matrix<4, 4> mat = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
    const ray_tracer::Ray ray2 = ray.Transform(mat);
    THEN("r2.origin = point(2,6,12)") { CHECK(ray2.GetOrigin() == ray_tracer::Point(2, 6, 12)); }
    THEN("r2.direction = vector(0,3,0)") { CHECK(ray2.GetDirection() == ray_tracer::Vec3(0, 3, 0)); }
  }
}

SCENARIO("5: A Sphere's default transformation") {
  GIVEN("s = Sphere()") {
    const ray_tracer::Sphere sphe(1);
    THEN("s.transform = identity_matrix") { CHECK(sphe.GetTransform() == ray_tracer::Matrix<4, 4>::IdentityMatrix()); }
  }
}
SCENARIO("5: Changing a Sphere's transformation") {
  GIVEN("s = Sphere(),t = translation(2,3,4)") {
    ray_tracer::Sphere sphe(1);
    const ray_tracer::Matrix<4, 4> mat = ray_tracer::Matrix<4, 4>::Translation(2, 3, 4);
    sphe.SetTransform(mat);
    THEN("s.transform = t") { CHECK(sphe.GetTransform() == mat); }
  }
}
SCENARIO("5: Intersecting a scaled Sphere with a ray") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s = Sphere()") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere sphe(1);
    sphe.SetTransform(ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2));
    THEN("xs = intersect(s,r)") {
      auto local_ray = ray.Transform(sphe.GetTransform().Inverse());
      auto xs_inter = sphe.LocalIntersect(local_ray);
      THEN("xs.count = 2") { CHECK(xs_inter.size() == 2); }
      THEN("xs[0].GetTime() = 3") { CHECK(xs_inter[0].GetTime() == 3); }
      THEN("xs[1].GetTime() = 7") { CHECK(xs_inter[1].GetTime() == 7); }
    }
  }
}
SCENARIO("5: Intersecting a translated Sphere with a ray") {
  GIVEN("r = Ray(point(0,0,-5), vector(0,0,1)),s = Sphere()") {
    const ray_tracer::Ray ray(ray_tracer::Point(0, 0, -5), ray_tracer::Vec3(0, 0, 1));
    ray_tracer::Sphere sphe(1);
    sphe.SetTransform(ray_tracer::Matrix<4, 4>::Translation(5, 0, 0));
    THEN("xs = intersect(s,r)") {
      auto local_ray = ray.Transform(sphe.GetTransform().Inverse());
      auto xs_inter = sphe.LocalIntersect(local_ray);
      THEN("xs.count = 0") { CHECK(xs_inter.empty()); }
    }
  }
}

}  // namespace ray_tracer
