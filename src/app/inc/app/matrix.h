#pragma once
#include <array>
#include <cmath>
#include <iostream>
#include "tuple.h"

namespace ray_tracer {
template <size_t Rows, size_t Cols>
class Matrix {
 public:
  constexpr Matrix() : matrix_() {}
  constexpr explicit Matrix(double value) {
    static_assert(Rows == 1 && Cols == 1, "Scalar initialization is only valid for 1x1 matrices.");
    matrix_[0][0] = value;
  }
  constexpr Matrix(std::initializer_list<std::initializer_list<double>> matrix) {
    if (matrix.size() != Rows) {
      throw std::invalid_argument("Matrix rows must match");
    }
    size_t row = 0;
    for (const auto& r : matrix) {
      if (r.size() != Cols) {
        throw std::invalid_argument("Matrix columns must match");
      }
      std::copy(r.begin(), r.end(), matrix_[row].begin());
      row++;
    }
  }
  explicit constexpr Matrix(const Tuple& tuple) {
    static_assert(Rows == 4 && Cols == 1, "Tuple must be 4x1");
    matrix_[0][0] = tuple.GetX();
    matrix_[1][0] = tuple.GetY();
    matrix_[2][0] = tuple.GetZ();
    matrix_[3][0] = tuple.GetW();
  }
  constexpr auto operator()(size_t row, size_t col) -> double& {
    if (row < Rows && col < Cols) {
      return matrix_[row][col];
    }
    throw std::invalid_argument("Matrix row or column out of bounds");
  }

  constexpr auto operator[](size_t row) -> std::array<double, Cols>& {
    if (row < Rows) {
      return matrix_[row];
    }
    throw std::invalid_argument("Matrix row out of bounds");
  }
  [[nodiscard]] static constexpr auto GetRows() -> size_t { return Rows; }
  [[nodiscard]] static constexpr auto GetCols() -> size_t { return Cols; }

  template <size_t R, size_t C, size_t R2, size_t C2>
  friend auto constexpr operator==(const Matrix<R, C>& lhs, const Matrix<R2, C2>& rhs) -> bool;  // NOLINT [redundant]
  template <size_t R, size_t C, size_t R2, size_t C2>
  friend auto constexpr operator!=(const Matrix<R, C>& lhs, const Matrix<R2, C2>& rhs) -> bool;  // NOLINT [redundant]
  template <size_t R, size_t C, size_t R2, size_t C2>
  friend auto constexpr operator*(const Matrix<R, C>& lhs, const Matrix<R2, C2>& rhs)  // NOLINT [redundant]
      -> Matrix<R, C2>;
  template <size_t R, size_t C>
  friend auto constexpr operator*(const Matrix<R, C>& lhs, const Tuple& tuple)  // NOLINT [redundant]
      -> Tuple;
  template <size_t R, size_t C>
  friend constexpr auto operator<<(std::ostream& output_stream, const Matrix<R, C>& matrix)  // NOLINT [redundant]
      -> std::ostream&;
  static constexpr auto IdentityMatrix() -> Matrix<Rows, Cols>;

  template <size_t R, size_t C>
  friend auto constexpr Transpose(const Matrix<R, C>& lhs) -> Matrix<C, R>;  // NOLINT [redundant]
  constexpr auto Determinant() -> double;
  constexpr auto MinorDeterminant() -> double;
  constexpr auto Minor(const size_t& row, const size_t& col) -> double;
  constexpr auto Cofactor(const size_t& row, const size_t& col) -> double;
  constexpr auto SpotSubMatrix(const size_t& row, const size_t& col) -> Matrix<Rows - 1, Cols - 1>;
  constexpr auto Invertible() -> bool;
  constexpr auto Inverse() -> Matrix<Rows, Cols>;
  static constexpr auto Scaling(const double& x_dir, const double& y_dir, const double& z_dir)
      -> ray_tracer::Matrix<4, 4>;
  static constexpr auto Scaling(const Tuple& tuple) -> ray_tracer::Matrix<4, 4>;
  static constexpr auto Translation(const double& x_dir, const double& y_dir, const double& z_dir)
      -> ray_tracer::Matrix<4, 4>;
  static constexpr auto Translation(const Tuple& tuple) -> ray_tracer::Matrix<4, 4>;

 private:
  std::array<std::array<double, Cols>, Rows> matrix_;
};

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::IdentityMatrix() -> Matrix<Rows, Cols> {
  static_assert(Rows == Cols, "Identity matrix must be square");
  Matrix<Rows, Cols> identity_matrix;
  for (size_t i = 0; i < Rows; i++) {
    for (size_t j = 0; j < Cols; j++) {
      if (i == j) {
        identity_matrix(i, j) = 1;
      } else {
        identity_matrix(i, j) = 0;
      }
    }
  }
  return identity_matrix;
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Determinant() -> double {
  static_assert(Rows == Cols, "Determinant matrix must be square");
  static_assert(Rows > 0, "Determinant matrix must be at least 1x1");
  double determinant = 0;

  if constexpr (Rows == 0) {
    determinant = 0;
  } else if constexpr (Rows == 1) {
    determinant = matrix_[0][0];
  } else if constexpr (Rows == 2) {
    determinant = matrix_[0][0] * matrix_[1][1] - matrix_[0][1] * matrix_[1][0];
  } else if constexpr (Rows < 2) {
    throw std::logic_error("Determinant cannot be calculated for matrix of size < 2");
  } else if constexpr (Rows > 1000) {
    determinant = 0;
    throw std::logic_error("Determinant cannot be calculated for matrix of size > 1000");
  } else {
    for (size_t col = 0; col < Cols; col++) {
      double sign = ((0 + col) % 2 == 0) ? 1.0 : -1.0;
      if constexpr (Rows <= 2) {
        determinant += sign * matrix_[0][col] * matrix_[0][0];
      } else {
        determinant += sign * matrix_[0][col] * SpotSubMatrix(0, col).Determinant();
      }
    }
  }
  return determinant;
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::MinorDeterminant() -> double {
  static_assert(Rows == Cols, "Determinant matrix must be square");
  return matrix_[0][0] * matrix_[1][1] - matrix_[0][1] * matrix_[1][0];
}
template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Minor(const size_t& row, const size_t& col) -> double {
  static_assert(Rows == Cols, "Determinant matrix must be square");
  static_assert(Rows >= 3, "Minor needs matrix must be >=3");
  if constexpr (Rows <= 2) {
    return matrix_[0][0];
  }
  if constexpr (Rows == 3) {
    return SpotSubMatrix(row, col).MinorDeterminant();
  }
  if constexpr (Rows == 1) {
    return 1;
  }
  if constexpr (Rows == 0) {
    return 1;
  }
  if constexpr (Rows == 0) {
    return 1;
  }
  return SpotSubMatrix(row, col).Determinant();
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Cofactor(const size_t& row, const size_t& col) -> double {
  static_assert(Rows == Cols, "Determinant matrix must be square");

  if (row >= Rows || col >= Cols) {
    throw std::invalid_argument("Row or column out of bounds");
  }

  if constexpr (Rows <= 2) {
    std::cerr << "Cofactor is not implemented for matrices of size 2 or less." << '\n';
    return 0.0;
  }
  return ((row + col) % 2 == 0 ? 1 : -1) * SpotSubMatrix(row, col).Determinant();
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::SpotSubMatrix(const size_t& row, const size_t& col) -> Matrix<Rows - 1, Cols - 1> {
  static_assert(Rows == Cols, "Determinant matrix must be square");
  // following doesn't actually let template know they cannot be 1 or 0;it doesnt compile for 1/0
  static_assert(Rows <= 1000 && Cols <= 1000, "Cannot create a submatrix for matrices smaller than 2x2.");
  Matrix<Rows - 1, Cols - 1> sub_matrix;
  if (row > Rows || col > Cols) {
    throw std::logic_error("Submatrix row or column out of bounds");
  }

  if constexpr (Rows == 1 || Rows == 0 || Rows == 2) {
    throw std::logic_error("submatrix of 1x1 is undefined");
  }
  size_t sub_row = 0;
  size_t sub_col = 0;
  for (size_t i = 0; i < Rows; i++) {
    for (size_t j = 0; j < Cols; j++) {
      if (j == col || i == row) {
        continue;
      }
      sub_matrix[sub_row][sub_col] = matrix_[i][j];
      sub_col++;
    }
    if (i == row) {
      continue;
    }
    sub_col = 0;
    sub_row++;
  }
  return sub_matrix;
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Invertible() -> bool {
  return Determinant() != 0;
}
template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Inverse() -> Matrix<Rows, Cols> {
  static_assert(Rows == Cols, "Determinant matrix must be square");
  if (!Invertible()) {
    throw std::logic_error("Matrix is not invertible");
  }
  Matrix<Rows, Cols> inverse;
  for (size_t i = 0; i < Rows; i++) {
    for (size_t j = 0; j < Cols; j++) {
      const double cofactor = Cofactor(i, j);
      inverse.matrix_[j][i] = cofactor / Determinant();
    }
  }
  return inverse;
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Translation(const double& x_dir, const double& y_dir, const double& z_dir)
    -> ray_tracer::Matrix<4, 4> {
  auto result = ray_tracer::Matrix<4, 4>::IdentityMatrix();
  result.matrix_[0][3] = x_dir;
  result.matrix_[1][3] = y_dir;
  result.matrix_[2][3] = z_dir;
  return result;
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Translation(const Tuple& tuple) -> ray_tracer::Matrix<4, 4> {
  return Translation(tuple.GetX(), tuple.GetY(), tuple.GetZ());
}
template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Scaling(const double& x_dir, const double& y_dir, const double& z_dir)
    -> ray_tracer::Matrix<4, 4> {
  auto result = ray_tracer::Matrix<4, 4>::IdentityMatrix();
  result.matrix_[0][0] = x_dir;
  result.matrix_[1][1] = y_dir;
  result.matrix_[2][2] = z_dir;
  return result;
}

template <size_t Rows, size_t Cols>
constexpr auto Matrix<Rows, Cols>::Scaling(const Tuple& tuple) -> ray_tracer::Matrix<4, 4> {
  return Scaling(tuple.GetX(), tuple.GetY(), tuple.GetZ());
}

template <size_t R, size_t C, size_t R2, size_t C2>
auto constexpr operator*(const Matrix<R, C>& lhs, const Matrix<R2, C2>& rhs) -> Matrix<R, C2> {
  static_assert(C == R2, "Matrix dimensions must match for multiplication");
  Matrix<R, C2> result;
  for (size_t i = 0; i < R; i++) {
    for (size_t j = 0; j < C2; j++) {
      double sum = 0;
      for (size_t k = 0; k < C; k++) {
        sum += lhs.matrix_[i][k] * rhs.matrix_[k][j];
      }
      result.matrix_[i][j] = sum;
    }
  }
  return result;
}
template <size_t R, size_t C>
auto constexpr operator*(const Matrix<R, C>& lhs, const Tuple& tuple) -> Tuple {
  static_assert(C == 4, "Matrix must be 4x4 for multiplication with Tuple");
  Matrix<R, 1> temp(tuple);
  Matrix<R, 1> temp2 = lhs * temp;
  return Tuple(temp2[0][0], temp2[1][0], temp2[2][0], temp2[3][0]);
}

template <size_t R, size_t C, size_t R2, size_t C2>
constexpr auto operator==(const Matrix<R, C>& lhs, const Matrix<R2, C2>& rhs) -> bool {
  if (R != R2 || C != C2) {
    return false;
  }
  for (size_t i = 0; i < R; ++i) {
    for (size_t j = 0; j < C; ++j) {
      if (!utilities::Equal(lhs.matrix_[i][j], rhs.matrix_[i][j])) {
        return false;
      }
    }
  }
  return true;
}
template <size_t Rows, size_t Cols, size_t Rows2, size_t Cols2>
constexpr auto operator!=(const Matrix<Rows, Cols>& lhs, const Matrix<Rows2, Cols2>& rhs) -> bool {
  return !(lhs == rhs);
}

template <size_t R, size_t C>
auto constexpr Transpose(const Matrix<R, C>& lhs) -> Matrix<C, R> {
  Matrix<C, R> result;
  for (size_t i = 0; i < R; i++) {
    for (size_t j = 0; j < C; j++) {
      result(j, i) = lhs.matrix_[i][j];
    }
  }
  return result;
}
template <size_t R, size_t C>
constexpr auto operator<<(std::ostream& output_stream, const Matrix<R, C>& matrix) -> std::ostream& {
  output_stream << "[\n";
  for (const auto& row : matrix.matrix_) {
    output_stream << "  [ ";
    for (const auto& elem : row) {
      output_stream << elem << " ";
    }
    output_stream << "]\n";
  }
  output_stream << "]";
  return output_stream;
}  // namespace ray_tracer
}  // namespace ray_tracer
