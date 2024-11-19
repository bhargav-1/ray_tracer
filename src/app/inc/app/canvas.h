#pragma once
#include <fstream>
#include <vector>
#include "color.h"
#include "point.h"

namespace ray_tracer {
struct Pixel {
  int x;
  int y;
  Color color;
};
class Canvas {
 public:
  // keeping it trunc- to not add everytime we run this program,
  // will delete the content and create new stuff
  Canvas(int width, int height) : width_(width), height_(height) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        Pixel const pxl{.x = x, .y = y, .color = {0, 0, 0}};
        pixels_.push_back(pxl);
      }
    }
  }
  void CanvasToPPM(const std::string& filename) noexcept {
    std::ofstream canvas_file{filename, std::ios::trunc};
    if (!canvas_file) {
      std::cout << "Unable to open file: " << filename << "\n";
    }

    canvas_file << "P3\n" << width_ << " " << height_ << "\n255 \n";
    const int max_line_length = 70;
    int current_line_length = 0;
    for (auto pxl : pixels_) {
      int const red = std::clamp(static_cast<int>(std::ceil(pxl.color.GetRed() * 255)), 0, 255);
      int const green = std::clamp(static_cast<int>(std::ceil(pxl.color.GetGreen() * 255)), 0, 255);
      int const blue = std::clamp(static_cast<int>(std::ceil(pxl.color.GetBlue() * 255)), 0, 255);

      std::string const pixel_data =
          std::to_string(red) + " " + std::to_string(green) + " " + std::to_string(blue) + " ";
      if (current_line_length + pixel_data.size() > max_line_length) {
        canvas_file << "\n";
        current_line_length = 0;
      }
      canvas_file << pixel_data;
      current_line_length += static_cast<int>(pixel_data.size());
    }
    canvas_file << "\n";
    canvas_file << "\n";
    canvas_file.close();
  }

  [[nodiscard]] auto GetWidth() const -> int { return width_; }
  [[nodiscard]] auto GetHeight() const -> int { return height_; }
  auto ColorAtPixel(int x_dir, int y_dir) -> Color {
    if (x_dir < width_ && y_dir < height_ && x_dir > 0 && y_dir > 0) {
      return pixels_[(width_ * y_dir) + x_dir].color;
    }
    return {0, 0, 0};
  }

  auto AllPixelsColorCheck(const Color& color) -> bool {
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        if (ColorAtPixel(x, y) != color) {
          return false;
        }
      }
    }
    return true;
  }
  void WritePixelColor(int x, int y, const Color& color) {
    if (x < width_ && x > 0 && y < height_ && y > 0) {
      pixels_[(width_ * y) + x].color = color;
    }
  }

  void PrintPixels() {
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        auto temp = pixels_[(width_ * y) + x].color.GetRed();
        if (temp == 1) {
          std::cout << "x: " << x << " y:" << y << ' ';
          std::cout << " red: " << pixels_[(width_ * y) + x].color.GetRed() << "\n";
        }
      }
    }
    std::cout << "------- " << '\n';
  }

 private:
  int width_;
  int height_;
  std::vector<Pixel> pixels_;
};
}  // namespace ray_tracer