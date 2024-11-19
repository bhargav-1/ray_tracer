#include <app/point.h>                   // for point
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for point
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <catch2/catch_tostring.hpp>     // for ReusableStringStream

// Behavior Driven Development Style Testing
// https://github.com/catchorg/Catch2/blob/devel/docs/test-cases-and-sections.md#bdd-style-test-cases

SCENARIO("A Tuple with w=1.0 is a point", "[point]") {
  GIVEN("w is 0") {
    THEN("its a vector") {
      ray_tracer::Tuple const tp_test(0.0, -1.3, 4.3, 0);
      CHECK(tp_test.IsPoint() == false);
    }
  }
  GIVEN("w is 1") {
    THEN("its a point") {
      ray_tracer::Tuple tp_test(0, 0, 1, 1);
      CHECK(tp_test.IsPoint() == true);
    }
  }
}

SCENARIO("point() creates tuples with w=1") {
  GIVEN("tuple with w 1") {
    THEN("tuple and point are same") {
      ray_tracer::Tuple test_tuple(1.2, 12, 2, 1);
      ray_tracer::Point test_point(1.2, 12, 2);
      CHECK(test_tuple == test_point);
    }
  }

  GIVEN("tuple with w 0") {
    THEN("tuple and point are not same") {
      ray_tracer::Tuple test_tuple(1.2, 12, 2, 0);
      ray_tracer::Point test_point(1.2, 12, 2);
      CHECK(test_tuple != test_point);
    }
  }
}
SCENARIO("vec3() creates tuples with w = 0") {
  GIVEN("tuple with w 1") {
    THEN("tuple and vec3 are same") {
      ray_tracer::Tuple test_tuple(1.2, 12, 2, 0);
      ray_tracer::Vec3 test_vec(1.2, 12, 2);
      CHECK(test_tuple == test_vec);
    }
  }

  GIVEN("tuple with w 0") {
    THEN("tuple and point are not same") {
      ray_tracer::Tuple test_tuple(1.2, 12, 2, 1);
      ray_tracer::Vec3 test_vec(1.2, 12, 2);
      CHECK(test_tuple != test_vec);
    }
  }
}

SCENARIO("add two tuples ") {
  GIVEN("one is tuple, other is vector") {
    THEN("adds up x+x,y+y,z+z") {
      ray_tracer::Tuple tuple1(1, 2, 3, 1);
      ray_tracer::Tuple tuple2(-1, 2, 33, 0);
      CHECK(tuple1 + tuple2 == ray_tracer::Tuple(0, 4, 36, 1));
    }
  }
  GIVEN("one is tuple, other is vector") {
    THEN("adds up x+x,y+y,z+z,w+w") {
      ray_tracer::Tuple tuple1(1, 2, 3, 0);
      ray_tracer::Point tuple2(-1, 0, 33);
      CHECK(tuple1 + tuple2 == ray_tracer::Tuple(0, 2, 36, 1));
    }
  }
  GIVEN("one is point, other is vector") {
    THEN("adds up x+x,y+y,z+z,w+w") {
      ray_tracer::Vec3 test_vec(1, 2, 3);
      ray_tracer::Point test_point(-1, 0, 33);
      test_point = test_point + test_vec;
      CHECK(test_point == ray_tracer::Tuple(0, 2, 36, 1));
    }
  }
}
SCENARIO("subtract two tuples ") {
  GIVEN("both are points") {
    THEN("adds up x+x,y+y,z+z,w+w") {
      ray_tracer::Point const tuple1(1, 2, 3);
      ray_tracer::Point const tuple2(-1, 0, 33);
      ray_tracer::Vec3 vec = tuple1 - tuple2;
      REQUIRE(vec.IsVector());
      CHECK(tuple1 - tuple2 == ray_tracer::Vec3(2, 2, -30));
    }
  }
  GIVEN("one is tuple, other is vector") {
    THEN("results in x-x,y-y,z-z") {
      ray_tracer::Point const test_point{3, 2, 1};
      ray_tracer::Vec3 const test_vector{5, 6, 7};
      CHECK(test_point - test_vector == ray_tracer::Point(-2, -4, -6));
    }
  }
  GIVEN("vector") {
    THEN("results in x-x,y-y,z-z") {
      ray_tracer::Vec3 const vec1{3, 2, 1};
      ray_tracer::Vec3 const vec2{5, 6, 7};
      CHECK(vec1 - vec2 == ray_tracer::Vec3(-2, -4, -6));
    }
  }
}

SCENARIO("negating tuples ") {
  GIVEN("for points") {
    THEN("returns tuple with -x,-y,-z") {
      ray_tracer::Tuple const tuple1(1, 2, 3, 4);
      CHECK(-tuple1 == ray_tracer::Tuple(-1, -2, -3, -4));
    }
  }
}
SCENARIO("scaling tuples ") {
  GIVEN("scale by 2 ") {
    THEN("returns tuple with 2x,2y,2z") {
      ray_tracer::Tuple const tuple1(1, 2, 3, 4);
      CHECK(tuple1 * 2 == ray_tracer::Tuple(2, 4, 6, 8));
    }
  }
  GIVEN("scale by 1/2 ") {
    THEN("returns tuple with x/2,y/2,z/2") {
      ray_tracer::Tuple const tuple1(1, 2, 3, 4);
      CHECK(tuple1 / 2 == ray_tracer::Tuple(0.5, 1, 1.5, 2));
    }
  }
  GIVEN("scale by 3 ") {
    THEN("returns tuple with x*3,y*3,z*3") {
      ray_tracer::Tuple const tuple1(1, 2, 3, 4);
      CHECK(3 * tuple1 == ray_tracer::Tuple(3, 6, 9, 12));
    }
  }
}
SCENARIO("Finding Vector Magnitude") {
  GIVEN("vector 0,0,1 ") {
    THEN("returns 1") {
      ray_tracer::Vec3 const vec(0, 0, 1);
      CHECK(vec.Magnitude() == 1);
    }
  }
  GIVEN("vector -1,-2,-3 ") {
    THEN("returns sqrt of 14") {
      ray_tracer::Vec3 const vec(-1, -2, -3);
      CHECK(vec.Magnitude() == std::sqrt(14));
    }
  }
  GIVEN("vector 1,2,3 ") {
    THEN("returns sqrt of 14") {
      ray_tracer::Vec3 const vec(-1, -2, -3);
      CHECK(vec.Normalize().Magnitude() == 1);
    }
  }
}

SCENARIO("Normalizing Vector") {
  GIVEN("vector 4,0,0 ") {
    THEN("returns vector 1,0,0") {
      ray_tracer::Vec3 const vec(4, 0, 0);
      CHECK(vec.Normalize() == ray_tracer::Vec3{1, 0, 0});
    }
  }
  GIVEN("vector 1,2,3 ") {
    THEN("returns vector 0.26726,0.53452,0.80178") {
      ray_tracer::Vec3 const vec(1, 2, 3);
      CHECK(vec.Normalize() == ray_tracer::Vec3{0.26726, 0.53452, 0.80178});
    }
  }
}
SCENARIO("Dot Product Vector") {
  GIVEN("vector (1,2,3) and (2,3,4)") {
    THEN("returns 20 as dot product") {
      ray_tracer::Vec3 const vec1(1, 2, 3);
      ray_tracer::Vec3 const vec2(2, 3, 4);
      CHECK(vec1.Dot(vec2) == 20);
    }
  }
}
SCENARIO("cross Product Vector") {
  GIVEN("vector (1,2,3) and (2,3,4)") {
    THEN("returns Vec3(-1,2,-1)") {
      ray_tracer::Vec3 const vec1(1, 2, 3);
      ray_tracer::Vec3 const vec2(2, 3, 4);
      CHECK(vec1.Cross(vec2) == ray_tracer::Vec3{-1, 2, -1});
    }
  }
  GIVEN("vector (1,2,3) and (2,3,4)") {
    THEN("returns Vec3(1,-2,1)") {
      ray_tracer::Vec3 const vec1(1, 2, 3);
      ray_tracer::Vec3 const vec2(2, 3, 4);
      CHECK(vec2.Cross(vec1) == ray_tracer::Vec3{1, -2, 1});
    }
  }
}
