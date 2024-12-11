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
  auto operator()(size_t row, size_t col) -> double& {
    if (row < Rows && col < Cols) {
      return matrix_[row][col];
    }
    throw std::invalid_argument("Matrix row or column out of bounds");
  }

  auto operator[](size_t row) -> std::array<double, Cols>& {
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

 private:
  std::array<std::array<double, Cols>, Rows> matrix_;
};  // namespace ray_tracer
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
}  // namespace ray_tracer
