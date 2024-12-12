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
SCENARIO("Given 2 identical matrices ") {
  GIVEN("A= B =M[4][4] = {{1,2,3,4},{5,6,7,8},{9,8,7,6},{5,4,3,2}}") {
    THEN("A==B") {
      ray_tracer::Matrix<4, 4> a_matrix{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 8, 7, 6}, {5, 4, 3, 2}};
      ray_tracer::Matrix<4, 4> b_matrix{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 8, 7, 6}, {5, 4, 3, 2}};
      CHECK(a_matrix == b_matrix);
    }
  }
}
SCENARIO("Given 2 non-identical matrices ") {
  GIVEN("A= B =M[4][4] = {{1,2,3,4},{5,6,7,8},{9,8,7,6},{5,4,3,2}}") {
    THEN("A!=B") {
      ray_tracer::Matrix<4, 4> a_matrix{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 8, 7, 6}, {5, 4, 5, 2}};
      ray_tracer::Matrix<4, 4> b_matrix{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 8, 7, 6}, {5, 4, 3, 2}};
      CHECK(a_matrix != b_matrix);
    }
  }
}
SCENARIO("Multiply 4x4 ") {
  GIVEN("A= B =M[4][4] = {{1,2,3,4},{5,6,7,8},{9,8,7,6},{5,4,3,2}}") {
    THEN("C=A*B") {
      ray_tracer::Matrix<4, 4> a_matrix{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 8, 7, 6}, {5, 4, 3, 2}};
      ray_tracer::Matrix<4, 4> b_matrix{{-2, 1, 2, 3}, {3, 2, 1, -1}, {4, 3, 6, 5}, {1, 2, 7, 8}};
      ray_tracer::Matrix<4, 4> const c_matrix{
          {20, 22, 50, 48}, {44, 54, 114, 108}, {40, 58, 110, 102}, {16, 26, 46, 42}};
      CHECK(c_matrix == a_matrix * b_matrix);
    }
  }
}
SCENARIO("Multiply 4x4 and tuple") {
  GIVEN("A= B =M[4][4] = {{1,2,3,4},{5,6,7,8},{9,8,7,6},{5,4,3,2}}") {
    THEN("C=A*B") {
      ray_tracer::Matrix<4, 4> a_matrix{{1, 2, 3, 4}, {2, 4, 4, 2}, {8, 6, 4, 1}, {0, 0, 0, 1}};
      ray_tracer::Tuple a_tuple(1, 2, 3, 1);
      ray_tracer::Tuple const c_tuple{18, 24, 33, 1};
      CHECK(c_tuple == a_matrix * a_tuple);
    }
  }
}
SCENARIO("Multiply 4x4 and Identity matrix") {
  GIVEN("A[4][4] = {{0,1,2,4},{1,2,4,8},{2,4,8,16},{4,8,16,32}}") {
    THEN("A=A*I") {
      ray_tracer::Matrix<4, 4> a_matrix{{0, 1, 2, 4}, {1, 2, 4, 8}, {2, 4, 8, 16}, {4, 8, 16, 32}};
      const auto identity_matrix = ray_tracer::Matrix<4, 4>::IdentityMatrix();
      CHECK(a_matrix == a_matrix * identity_matrix);
    }
  }
}
SCENARIO("Multiply tuple and Identity matrix") {
  GIVEN("tuple A = {1,2,3,4}") {
    THEN("A=A*I") {
      Tuple a_tuple(1, 2, 3, 4);
      const auto identity_matrix = ray_tracer::Matrix<4, 4>::IdentityMatrix();
      CHECK(a_tuple == identity_matrix * a_tuple);
    }
  }
}
SCENARIO("Transposing a Matrix") {
  GIVEN("Matrix A[4][4] = {{0,9,3,0},{9,8,0,8},{1,8,5,3},{0,0,5,8}}") {
    THEN("Transpose(A) = {{0,9,1,0},{9,8,8,0},{3,0,5,5},{0,8,3,8}}") {
      ray_tracer::Matrix<4, 4> temp_matrix{{0, 9, 3, 0}, {9, 8, 0, 8}, {1, 8, 5, 3}, {0, 0, 5, 8}};
      ray_tracer::Matrix<4, 4> const transpose_matrix{{0, 9, 1, 0}, {9, 8, 8, 0}, {3, 0, 5, 5}, {0, 8, 3, 8}};
      CHECK(transpose_matrix == Transpose(temp_matrix));
    }
  }
}
  SCENARIO("Transposing the Identity Matrix") {
  GIVEN("Matrix A = IdentityMatrix<4,4>") {
    THEN("A=A*I") {
      const auto identity_matrix = ray_tracer::Matrix<4, 4>::IdentityMatrix();
      CHECK(identity_matrix == Transpose(identity_matrix));
    }
  }
}
}  // namespace ray_tracer
