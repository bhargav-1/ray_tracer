#include <app/matrix.h>                  // for point
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <catch2/catch_tostring.hpp>     // for ReusableStringStream

// Behavior Driven Development Style Testing
// https://github.com/catchorg/Catch2/blob/devel/docs/test-cases-and-sections.md#bdd-style-test-cases
namespace ray_tracer {
SCENARIO("Given 4 x4 matrix ") {
  GIVEN("M[4][4] = {{1,2,3,4},{5.5,6.5,7.5,8.5},{9,10,11,12},{13.5,14.5,15.5,16.5}}") {
    THEN("M[0,0] = 1") {
      ray_tracer::Matrix<4, 4> test_matrix(
          {{1, 2, 3, 4}, {5.5, 6.5, 7.5, 8.5}, {9, 10, 11, 12}, {13.5, 14.5, 15.5, 16.5}});
      CHECK(test_matrix(0, 0) == 1);
      CHECK(test_matrix[0][0] == 1);
      CHECK(test_matrix[0][3] == 4);
      CHECK(test_matrix[1][0] == 5.5);
      CHECK(test_matrix[1][2] == 7.5);
      CHECK(test_matrix[2][2] == 11);
      CHECK(test_matrix[3][0] == 13.5);
      CHECK(test_matrix[3][2] == 15.5);
    }
  }
}
SCENARIO("Given 3 x 3 matrix ") {
  GIVEN("M[3][3] = {{-3,5,0},{1,-2,-7},{0,1,1}}") {
    THEN("M[0,0] = -3") {
      ray_tracer::Matrix<3, 3> test_matrix{{-3, 5, 0}, {1, -2, -7}, {0, 1, 1}};
      CHECK(test_matrix(0, 0) == -3);
      CHECK(test_matrix[0][0] == -3);
      CHECK(test_matrix[1][1] == -2);
      CHECK(test_matrix[2][2] == 1);
    }
  }
}
SCENARIO("Given 2 x 2 matrix ") {
  GIVEN("M[2][2] = {{-3,5},{1,-2}}") {
    THEN("M[0,0] = -3") {
      ray_tracer::Matrix<2, 2> test_matrix{{-3, 5}, {1, -2}};
      CHECK(test_matrix(0, 0) == -3);
      CHECK(test_matrix[0][0] == -3);
      CHECK(test_matrix[1][1] == -2);
      CHECK(test_matrix[0][1] == 5);
    }
  }
}
}  // namespace ray_tracer
