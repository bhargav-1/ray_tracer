#pragma once
#include <array>
#include <cmath>
#include <iostream>

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

 private:
  std::array<std::array<double, Cols>, Rows> matrix_;
};  // namespace ray_tracer
}  // namespace ray_tracer
