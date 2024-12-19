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
SCENARIO("Finding Determinant of 2x2 matrix") {
  GIVEN("Matrix A = {{1,5},{-3,2}}") {
    THEN("Determinant(A) = 17") {
      ray_tracer::Matrix<2, 2> temp_matrix{{1, 5}, {-3, 2}};
      CHECK(temp_matrix.MinorDeterminant() == 17);
    }
  }
}
SCENARIO("Spotting Submatrix of 3x3 matrix") {
  GIVEN("Matrix A = {{1,5,0},{-3,2,7},{0,6,-3}}") {
    THEN("Submatrix(A,0,2) = {{5,0},{-3,7}}") {
      ray_tracer::Matrix<3, 3> temp_matrix{{1, 5, 0}, {-3, 2, 7}, {0, 6, -3}};
      ray_tracer::Matrix<2, 2> const sub_matrix = temp_matrix.SpotSubMatrix(0, 2);
      ray_tracer::Matrix<2, 2> const expected_matrix{{-3, 2}, {0, 6}};
      CHECK(sub_matrix == expected_matrix);
    }
  }
  GIVEN("Matrix A = {{-6,1,1,6},{-8,5,8,6},{-1,0,8,2},{-7,1,-1,1}}") {
    THEN("Submatrix(A,2,1) = {{-6,1,6},{-8,8,6},{-7,-1,1}}") {
      ray_tracer::Matrix<4, 4> temp_matrix{{-6, 1, 1, 6}, {-8, 5, 8, 6}, {-1, 0, 8, 2}, {-7, 1, -1, 1}};
      ray_tracer::Matrix<3, 3> const sub_matrix = temp_matrix.SpotSubMatrix(2, 1);
      ray_tracer::Matrix<3, 3> const expected_matrix{{-6, 1, 6}, {-8, 8, 6}, {-7, -1, 1}};
      CHECK(sub_matrix == expected_matrix);
    }
  }
}
SCENARIO("Computing minor") {
  GIVEN("Matrix A[3][3] ={{3,5,0},{2,-1,-7},{6,-1,5}}") {
    THEN("Minor(1,0) = 25") {
      ray_tracer::Matrix<3, 3> temp_matrix{{3, 5, 0}, {2, -1, -7}, {6, -1, 5}};
      CHECK(temp_matrix.Minor(1, 0) == 25);
    }
  }
}
SCENARIO("Computing COfactor") {
  GIVEN("Matrix A[3][3] ={{3,5,0},{2,-1,-7},{6,-1,5}}") {
    THEN("Cofactor(1,0) = -12") {
      ray_tracer::Matrix<3, 3> temp_matrix{{3, 5, 0}, {2, -1, -7}, {6, -1, 5}};
      CHECK(temp_matrix.Cofactor(1, 0) == -25);
      AND_THEN("Cofactor(0,0) = -12") { CHECK(temp_matrix.Cofactor(0, 0) == -12); }
      AND_THEN("Cofactor(1,0) = 25") { CHECK(temp_matrix.Cofactor(1, 0) == -25); }
    }
  }
}
SCENARIO("Calculating Matrix Determinant") {
  GIVEN("MATRIX A ={{1,2,6},{-5,8,-4},{2,6,4}}") {
    THEN("Determinant(A) = -196") {
      ray_tracer::Matrix<3, 3> temp_matrix{{1, 2, 6}, {-5, 8, -4}, {2, 6, 4}};
      CHECK(temp_matrix.Cofactor(0, 0) == 56);
      AND_THEN("Cofactor(0,1) = 12") { CHECK(temp_matrix.Cofactor(0, 1) == 12); }
      AND_THEN("Cofactor(0,2) = -46") { CHECK(temp_matrix.Cofactor(0, 2) == -46); }
      AND_THEN("Determinant(A) = -196") { CHECK(temp_matrix.Determinant() == -196); }
    }
  }
  GIVEN("MATRIX A ={{-2,-8,3,5},{-3,1,7,3},{1,2,-9,6},{-6,7,7,-9}}") {
    THEN("Determinant(A) = -4074") {
      ray_tracer::Matrix<4, 4> temp{{-2, -8, 3, 5}, {-3, 1, 7, 3}, {1, 2, -9, 6}, {-6, 7, 7, -9}};
      CHECK(temp.Cofactor(0, 0) == 690);
      AND_THEN("Cofactor(0,1) = 447") { CHECK(temp.Cofactor(0, 1) == 447); }
      AND_THEN("Cofactor(0,2) = 210") { CHECK(temp.Cofactor(0, 2) == 210); }
      AND_THEN("Cofactor(0,3) = 51") { CHECK(temp.Cofactor(0, 3) == 51); }
      AND_THEN("Determinant(A) = -4071") { CHECK(temp.Determinant() == -4071); }
    }
  }
}
SCENARIO("Testing invertible matrix") {
  GIVEN("A = {{6,4,4,4},{5,5,7,6},{4,-9,3,-7},{9,1,7,-6}}") {
    THEN("Determinant(A) = -2120") {
      ray_tracer::Matrix<4, 4> temp{{6, 4, 4, 4}, {5, 5, 7, 6}, {4, -9, 3, -7}, {9, 1, 7, -6}};
      CHECK(temp.Determinant() == -2120);
      AND_THEN("A is invertible") { CHECK(temp.Invertible()); }
    }
  }
  GIVEN("A = {{-4,2,-2,-3},{9,6,2,6},{0,-5,1,-5},{0,0,0,0}}") {
    THEN("Determinant(A) = 0") {
      ray_tracer::Matrix<4, 4> temp{{-4, 2, -2, -3}, {9, 6, 2, 6}, {0, -5, 1, -5}, {0, 0, 0, 0}};
      CHECK(temp.Determinant() == 0);
      AND_THEN("A is not invertible") { CHECK(!temp.Invertible()); }
    }
  }
}

SCENARIO("Calculating the Inverse Matrix") {
  GIVEN("A = {-5,2,6,-8,1,-5,1,8,7,7,-6,-7,1,-3,7,4,9}") {
    THEN(
        "Inverse(A) = "
        "{{0.21805,0.45113,0.24060,-0.04511},{-0.80827,-1.45677,-0.44361,0.52068},{-0.07895,-0.22368,-0.05263,0.19737},"
        "{0.52256,0.85098,0.24060,-0.10526}}") {
      ray_tracer::Matrix<4, 4> temp{{-5, 2, 6, -8}, {1, -5, 1, 8}, {7, 7, -6, -7}, {1, -3, 7, 4}};
      ray_tracer::Matrix<4, 4> const inverse_matrix{{0.21805, 0.45113, 0.24060, -0.04511},
                                                    {-0.80827, -1.45677, -0.44361, 0.52068},
                                                    {-0.07895, -0.22368, -0.05263, 0.19737},
                                                    {-0.52256, -0.81391, -0.30075, 0.30639}};
      CHECK(temp.Determinant() == 532);
      AND_THEN("cofactor of A(2,3) is -160") { CHECK(temp.Cofactor(2, 3) == -160); }
      AND_THEN("inverse of A(3,2) is -160/532") { CHECK(utilities::Equal(temp.Inverse()(3, 2), (-160.0 / 532))); }
      AND_THEN("cofactor of A(3,2) is 105") { CHECK(temp.Cofactor(3, 2) == 105); }
      AND_THEN("inverse of A(2,3) is 105/532") { CHECK(utilities::Equal(temp.Inverse()(2, 3), (105.0 / 532))); }
      AND_THEN("Inverse(A) = inverse_matrix") { CHECK(temp.Inverse() == inverse_matrix); }
    }
  }
}
SCENARIO("Multiplying a product by its inverse") {
  GIVEN(
      "A ={{3,-9,7,3},{3,-8,2,-9},{-4,4,4,1},{-6,5,-1,1}}, B ={{8,2,2,2},{3,-1,7,0},{7,0,5,4},{6,-2,0,5}}, C = A * B") {
    THEN("C = A * B") {
      ray_tracer::Matrix<4, 4> a_matrix{{3, -9, 7, 3}, {3, -8, 2, -9}, {-4, 4, 4, 1}, {-6, 5, -1, 1}};
      ray_tracer::Matrix<4, 4> b_matrix{{8, 2, 2, 2}, {3, -1, 7, 0}, {7, 0, 5, 4}, {6, -2, 0, 5}};
      ray_tracer::Matrix<4, 4> const c_matrix{
          {64, 9, -22, 49}, {-40, 32, -40, -31}, {14, -14, 40, 13}, {-34, -19, 18, -11}};
      CHECK(a_matrix * b_matrix == c_matrix);
      AND_THEN("C * inverse(B) = A") { CHECK(c_matrix * (b_matrix.Inverse()) == a_matrix); }
    }
  }
}
SCENARIO("inverse of Identity Matrix") {
  GIVEN("A = Identity Matrix") {
    THEN("Inverse(A) = A") {
      ray_tracer::Matrix<4, 4> a_matrix{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
      CHECK(a_matrix.Inverse() == a_matrix);
    }
  }
}
SCENARIO("Multiplying a matrix with its Inverse") {
  GIVEN("A ={{3,-9,7,3},{3,-8,2,-9},{-4,4,4,1},{-6,5,-1,1}}") {
    THEN("A * inverse(A) = I") {
      ray_tracer::Matrix<4, 4> a_matrix{{3, -9, 7, 3}, {3, -8, 2, -9}, {-4, 4, 4, 1}, {-6, 5, -1, 1}};
      ray_tracer::Matrix<4, 4> const identity_matrix{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
      CHECK(a_matrix * a_matrix.Inverse() == identity_matrix);
    }
  }
}
SCENARIO("inverse of transpose of matrix and transpose of Inverse of matrix") {
  GIVEN("A ={{3,-9,7,3},{3,-8,2,-9},{-4,4,4,1},{-6,5,-1,1}}") {
    THEN("inverse(transpose(A)) = transpose(inverse(A))") {
      ray_tracer::Matrix<4, 4> a_matrix{{3, -9, 7, 3}, {3, -8, 2, -9}, {-4, 4, 4, 1}, {-6, 5, -1, 1}};
      CHECK(Transpose(a_matrix.Inverse()) == Transpose(a_matrix).Inverse());
    }
  }
}
  SCENARIO("Mutiplying almost I with tuple ") {
  GIVEN("A ={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,2}}, b = {1,2,3,4}") {
    THEN("A * b = {1,2,3,4}") {
      ray_tracer::Matrix<4, 4> a_matrix{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 2}};
      ray_tracer::Tuple b_tuple{1, 2, 3, 4};
      ray_tracer::Tuple c_tuple{1, 2, 3, 8};
      CHECK(a_matrix * b_tuple != b_tuple);
      AND_THEN("A * b = {1,2,3,8}") { CHECK(a_matrix * b_tuple == c_tuple); }
    }
  }
}

}  // namespace ray_tracer
