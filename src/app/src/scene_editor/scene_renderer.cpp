#include "app/scene_editor/scene_renderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string_view>
#include <utility>

#include "app/ray_tracing_engine.h"

namespace ray_tracer::scene_editor {
namespace {

auto PreviewDimensionFor(const int full_width, const int full_height) -> std::pair<int, int> {
  int height = std::max(static_cast<int>(std::lround(
                            static_cast<double>(full_height) / SceneRenderer::kPreviewDivisor)),
                        20);
  int width = static_cast<int>(std::lround((static_cast<double>(height) * full_width) / full_height));
  if (width < 40) {
    width = 40;
    height = static_cast<int>(std::lround((static_cast<double>(width) * full_height) / full_width));
  }
  return {std::max(width, 1), std::max(height, 1)};
}

auto PreviewSamplePixel(const int preview_pixel, const int preview_size, const int full_size) -> int {
  const double block_start = static_cast<double>(preview_pixel) * full_size / preview_size;
  const double block_end = static_cast<double>(preview_pixel + 1) * full_size / preview_size;
  const auto middle = static_cast<int>(std::floor((block_start + block_end) * 0.5));
  return std::clamp(middle, 0, full_size - 1);
}

auto LogRenderSamples(const scene_runtime::CompiledScene& scene,
                      const bool preview,
                      const int output_width,
                      const int output_height,
                      const int full_width,
                      const int full_height) -> void {
  auto camera = scene.camera;
  struct SamplePoint {
    std::string_view label;
    int x;
    int y;
  };

  const std::array samples{
      SamplePoint{"top_left", 0, 0},
      SamplePoint{"center", output_width / 2, output_height / 2},
      SamplePoint{"bottom_right", output_width - 1, output_height - 1},
  };

  std::fprintf(stderr,
               "[scene_editor] renderer samples mode=%s output=%dx%d full=%dx%d objects=%zu lights=%zu\n",
               preview ? "preview" : "full",
               output_width,
               output_height,
               full_width,
               full_height,
               scene.world.Objects().size(),
               scene.world.Lights().size());
  for (const auto& sample : samples) {
    const int output_x = std::clamp(sample.x, 0, output_width - 1);
    const int output_y = std::clamp(sample.y, 0, output_height - 1);
    const int source_x = preview ? PreviewSamplePixel(output_x, output_width, full_width) : output_x;
    const int source_y = preview ? PreviewSamplePixel(output_y, output_height, full_height) : output_y;
    const auto ray = camera.GetRayForPixel(source_x, source_y);
    const auto& origin = ray.GetOrigin();
    const auto& direction = ray.GetDirection();
    std::fprintf(stderr,
                 "[scene_editor] renderer sample mode=%s label=%s output_pixel=[%d %d] source_pixel=[%d %d] "
                 "origin=[%.6f %.6f %.6f] direction=[%.6f %.6f %.6f]\n",
                 preview ? "preview" : "full",
                 sample.label.data(),
                 output_x,
                 output_y,
                 source_x,
                 source_y,
                 origin.GetX(),
                 origin.GetY(),
                 origin.GetZ(),
                 direction.GetX(),
                 direction.GetY(),
                 direction.GetZ());
  }
}

}  // namespace

auto SceneRenderer::Render(const scene_runtime::CompiledScene& scene,
                             const bool preview,
                             std::atomic<bool>* cancel,
                             std::atomic<int>* progress) const -> RenderPixels {
  auto camera = scene.camera;
  const int full_width = camera.GetHorizontalSize();
  const int full_height = camera.GetVerticalSize();
  int width = full_width;
  int height = full_height;
  if (preview) {
    const auto [preview_width, preview_height] = PreviewDimensionFor(full_width, full_height);
    width = preview_width;
    height = preview_height;
  }

  RenderPixels image;
  image.width = width;
  image.height = height;
  image.pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

  LogRenderSamples(scene, preview, width, height, full_width, full_height);

  if (progress != nullptr) {
    progress->store(0);
  }
  for (int y = 0; y < height; ++y) {
    if (cancel != nullptr && cancel->load()) {
      return {};
    }
    for (int x = 0; x < width; ++x) {
      if (cancel != nullptr && cancel->load()) {
        return {};
      }
      auto& output = image.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                                  static_cast<std::size_t>(x)];
      if (!preview) {
        auto ray = camera.GetRayForPixel(x, y);
        output = ray_tracer::ColorAt(scene.world, ray);
        continue;
      }

      const int source_x = PreviewSamplePixel(x, width, full_width);
      const int source_y = PreviewSamplePixel(y, height, full_height);
      auto ray = camera.GetRayForPixel(source_x, source_y);
      output = ray_tracer::ColorAt(scene.world, ray);
    }
    if (progress != nullptr) {
      progress->store(((y + 1) * 100) / height);
    }
  }
  return image;
}

auto SceneRenderer::ToRgb8(const RenderPixels& image) -> std::vector<unsigned char> {
  std::vector<unsigned char> rgb(static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height) * 3);
  for (std::size_t i = 0; i < image.pixels.size(); ++i) {
    const auto& color = image.pixels[i];
    rgb[(i * 3) + 0] = static_cast<unsigned char>(std::clamp(color.GetRed(), 0.0, 1.0) * 255.0);
    rgb[(i * 3) + 1] = static_cast<unsigned char>(std::clamp(color.GetGreen(), 0.0, 1.0) * 255.0);
    rgb[(i * 3) + 2] = static_cast<unsigned char>(std::clamp(color.GetBlue(), 0.0, 1.0) * 255.0);
  }
  return rgb;
}

}  // namespace ray_tracer::scene_editor
