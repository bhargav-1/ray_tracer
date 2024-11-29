#pragma once
#include <complex>
#include <fstream>

namespace utilities {
constexpr double kEpsilon = 0.0001;
constexpr auto Equal(double left, double right) -> bool { return (std::abs(left - right) < kEpsilon); }

inline auto GetLines(int start_line_number, int end_line_number, const std::string& filename) -> std::string {
  std::ifstream file(filename);
  std::string lines;
  std::string line;

  int current_line = 0;
  while (current_line < start_line_number - 1 && std::getline(file, line)) {
    current_line++;
  }
  while (current_line < end_line_number && std::getline(file, line)) {
    lines += line + '\n';
    current_line++;
  }
  lines.pop_back();
  return lines;
}

}  // namespace utilities
