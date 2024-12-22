#include <app/matrix.h>                  // for matrix
#include <app/point.h>                   // for tuple
#include <app/tuple.h>                   // for tuple
#include <app/vec3.h>                    // for vector
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
// will use once ported to cpp20/ubuntu24
// #include <numbers>                       // for numbers
#include <cmath>

namespace ray_tracer {
SCENARIO("Multiply by translation") {
  GIVEN("Transform = translation{5,-3,2}, p = point(-3,4,5)") {
    THEN("Transform * p = point(2,1,7)") {
      auto transform = ray_tracer::Matrix<4, 4>::Translation(5, -3, 2);
      ray_tracer::Point p = ray_tracer::Point(-3, 4, 5);
      CHECK(transform * p == ray_tracer::Point(2, 1, 7));
    }
  }
}
SCENARIO("Multiply by inverse translation") {
  GIVEN("Transform = translation{5,-3,2}, p = point(-3,4,5)") {
    THEN("Transform * p = point(2,1,7)") {
      auto transform = ray_tracer::Matrix<4, 4>::Translation(5, -3, 2);
      ray_tracer::Point p = ray_tracer::Point(-3, 4, 5);
      CHECK(transform * p == ray_tracer::Point(2, 1, 7));
    }
  }
}
SCENARIO("Translation does not affect vectors") {
  GIVEN("Transform = translation{5,-3,2}, v = vector(-3,4,5)") {
    THEN("Transform * v = vector(-3,4,5)") {
      auto transform = ray_tracer::Matrix<4, 4>::Translation(5, -3, 2);
      ray_tracer::Vec3 v(-3, 4, 5);
      CHECK(transform * v == v);
    }
  }
}
SCENARIO("Scaling a point") {
  GIVEN("Transform = scaling{2,3,4}, p = point(-4,6,8)") {
    THEN("Transform * p = point(-8,18,32)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
      ray_tracer::Point p(-4, 6, 8);
      CHECK(transform * p == ray_tracer::Point(-8, 18, 32));
    }
  }
}

SCENARIO("Scaling a vector") {
  GIVEN("Transform = scaling{2,3,4}, v = vector(-4,6,8)") {
    THEN("Transform * v = vector(-8,18,32)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
      ray_tracer::Vec3 v(-4, 6, 8);
      CHECK(transform * v == ray_tracer::Vec3(-8, 18, 32));
    }
  }
}

SCENARIO("Multiply by inverse of scaling matrix") {
  GIVEN("Transform = scaling{2,3,4}, v = vector(-4,6,8)") {
    THEN("Transform * v = vector(-2,2,2)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(2, 3, 4);
      auto inv = transform.Inverse();
      ray_tracer::Vec3 v(-4, 6, 8);
      CHECK(inv * v == ray_tracer::Vec3(-2, 2, 2));
    }
  }
}
SCENARIO("Reflection is scaling by a negative value") {
  GIVEN("Transform = scaling{-1,1,1}, p = point(2,3,4)") {
    THEN("Transform * p = point(-2,3,4)") {
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(-1, 1, 1);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(-2, 3, 4));
    }
  }
}
SCENARIO("Rotating a point around the x axis") {
  GIVEN("p = point(0,1,0),half_quarter = rotation_x(pi/4),full_quarter = rotation_x(pi/2)") {
    THEN("half_quarter * p = point(0,sqrt(2)/2,sqrt(2)/2)") {
      ray_tracer::Point p(0, 1, 0);
      auto half_quarter = ray_tracer::Matrix<4, 4>::RotationX(M_PI / 4);
      CHECK(half_quarter * p == ray_tracer::Point(0, std::sqrt(2) / 2, std::sqrt(2) / 2));
      auto full_quarter = ray_tracer::Matrix<4, 4>::RotationX(M_PI / 2);
      CHECK(full_quarter * p == ray_tracer::Point(0, 0, 1));
    }
  }
}

SCENARIO("inverse of an x-rotation rotates in opp direction") {
  GIVEN("p = point(0,1,0),half_quarter = rotation_x(pi/4),full_quarter = rotation_x(pi/2)") {
    THEN("half_quarter * p = point(0,sqrt(2)/2,sqrt(2)/2)") {
      ray_tracer::Point p(0, 1, 0);
      auto half_quarter = ray_tracer::Matrix<4, 4>::RotationX(M_PI / 4);
      auto inv = half_quarter.Inverse();
      CHECK(inv * p == ray_tracer::Point(0, std::sqrt(2) / 2, -std::sqrt(2) / 2));
    }
  }
}
SCENARIO("Rotating a point around the y axis") {
  GIVEN("p = point(0,0,1),half_quarter = rotation_y(pi/4),full_quarter = rotation_y(pi/2)") {
    THEN("half_quarter * p = point(sqrt(2)/2,0,sqrt(2)/2)") {
      ray_tracer::Point p(0, 0, 1);
      auto half_quarter = ray_tracer::Matrix<4, 4>::RotationY(M_PI / 4);
      CHECK(half_quarter * p == ray_tracer::Point(std::sqrt(2) / 2, 0, std::sqrt(2) / 2));
      auto full_quarter = ray_tracer::Matrix<4, 4>::RotationY(M_PI / 2);
      CHECK(full_quarter * p == ray_tracer::Point(1, 0, 0));
    }
  }
}

SCENARIO("Rotating a point around the z axis") {
  GIVEN("p = point(0,1,0),half_quarter = rotation_z(pi/4),full_quarter = rotation_z(pi/2)") {
    THEN("half_quarter * p = point(-sqrt(2)/2,sqrt(2)/2,0)") {
      ray_tracer::Point p(0, 1, 0);
      auto half_quarter = ray_tracer::Matrix<4, 4>::RotationZ(M_PI / 4);
      CHECK(half_quarter * p == ray_tracer::Point(-std::sqrt(2) / 2, std::sqrt(2) / 2, 0));
      auto full_quarter = ray_tracer::Matrix<4, 4>::RotationZ(M_PI / 2);
      CHECK(full_quarter * p == ray_tracer::Point(-1, 0, 0));
    }
  }
}
SCENARIO("A shearing transformation moves x in proportion to y") {
  GIVEN("transform = shearing(1,0,0,0,0,0), p = point(2,3,4)") {
    THEN("transform * p = point(5,3,4)") {
      ray_tracer::shear::ShearX shear_x{1, 0};
      ray_tracer::shear::ShearY shear_y{0, 0};
      ray_tracer::shear::ShearZ shear_z{0, 0};
      auto transform = ray_tracer::Matrix<4, 4>::Shearing(shear_x, shear_y, shear_z);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(5, 3, 4));
    }
  }
}
SCENARIO("A shearing transformation moves y in proportion to x") {
  GIVEN("transform = shearing(0,0,1,0,0,0), p = point(2,3,4)") {
    THEN("transform * p = point(6,3,4)") {
      ray_tracer::shear::ShearX shear_x{0, 0};
      ray_tracer::shear::ShearY shear_y{1, 0};
      ray_tracer::shear::ShearZ shear_z{0, 0};
      auto transform = ray_tracer::Matrix<4, 4>::Shearing(shear_x, shear_y, shear_z);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(2, 5, 4));
    }
  }
}

SCENARIO("A shearing transformaiton moves y in proportion to z") {
  GIVEN("transform = shearing(0,0,0,1,0,0), p = point(2,3,4)") {
    THEN("transform * p = point(2,7,4)") {
      ray_tracer::shear::ShearX shear_x{0, 0};
      ray_tracer::shear::ShearY shear_y{0, 1};
      ray_tracer::shear::ShearZ shear_z{0, 0};
      auto transform = ray_tracer::Matrix<4, 4>::Shearing(shear_x, shear_y, shear_z);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(2, 7, 4));
    }
  }
}

SCENARIO("A shearing transformation moves z in proportion to x") {
  GIVEN("transform = shearing(0,0,0,0,1,0), p = point(2,3,4)") {
    THEN("transform * p = point(2,3,6)") {
      ray_tracer::shear::ShearX shear_x{0, 0};
      ray_tracer::shear::ShearY shear_y{0, 0};
      ray_tracer::shear::ShearZ shear_z{1, 0};
      auto transform = ray_tracer::Matrix<4, 4>::Shearing(shear_x, shear_y, shear_z);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(2, 3, 6));
    }
  }
}
SCENARIO("A shearing transformation moves z in proportion to y") {
  GIVEN("transform = shearing(0,0,0,0,0,1), p = point(2,3,4)") {
    THEN("transform * p = point(2,3,7)") {
      ray_tracer::shear::ShearX shear_x{0, 0};
      ray_tracer::shear::ShearY shear_y{0, 0};
      ray_tracer::shear::ShearZ shear_z{0, 1};
      auto transform = ray_tracer::Matrix<4, 4>::Shearing(shear_x, shear_y, shear_z);
      ray_tracer::Point p(2, 3, 4);
      CHECK(transform * p == ray_tracer::Point(2, 3, 7));
    }
  }
}

SCENARIO("Individual transformations are applied in sequence") {
  GIVEN("p = point(1,0,1), A = rotation_x(pi/2), B = scaling(5,5,5), C = translation(10,5,7)") {
    THEN("C * B * A * p = point(15,0,7)") {
      ray_tracer::Point p(1, 0, 1);
      auto a = ray_tracer::Matrix<4, 4>::RotationX(M_PI / 2);
      auto b = ray_tracer::Matrix<4, 4>::Scaling(5, 5, 5);
      auto c = ray_tracer::Matrix<4, 4>::Translation(10, 5, 7);
      auto p2 = a * p;
      CHECK(p2 == ray_tracer::Point(1, -1, 0));
      auto p3 = b * p2;
      CHECK(p3 == ray_tracer::Point(5, -5, 0));
      auto p4 = c * p3;
      CHECK(p4 == ray_tracer::Point(15, 0, 7));
    }
  }
}
SCENARIO("Chained transformations must be applied in reverse order") {
  GIVEN("p = point(1,0,1), A = rotation_x(pi/2), B = scaling(5,5,5), C = translation(10,5,7)") {
    THEN("C * B * A * p =point(15,0,7)") {
      ray_tracer::Point p(1, 0, 1);
      auto a = ray_tracer::Matrix<4, 4>::RotationX(M_PI / 2);
      auto b = ray_tracer::Matrix<4, 4>::Scaling(5, 5, 5);
      auto c = ray_tracer::Matrix<4, 4>::Translation(10, 5, 7);
      auto t = c * b * a;
      CHECK(t * p == ray_tracer::Point(15, 0, 7));
    }
  }
}
}  // namespace ray_tracer
