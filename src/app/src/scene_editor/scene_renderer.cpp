#include "app/scene_editor/scene_renderer.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "app/rendering/ray_tracing_engine.h"
#include "app/rendering/render_rows.h"
#include "app/scene/scene_compiler.h"

namespace ray_tracer::scene_editor {
namespace {

auto PreviewDimensionFor(const int full_width, const int full_height) -> std::pair<int, int> {
  int height =
      std::max(static_cast<int>(std::lround(static_cast<double>(full_height) / SceneRenderer::kPreviewDivisor)), 20);
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

}  // namespace

auto SceneRenderer::Render(const scene_runtime::CompiledScene& scene,
                           const bool preview,
                           std::atomic<bool>* cancel,
                           std::atomic<int>* progress) -> RenderPixels {
  const auto& camera = scene.camera;
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

  for (const auto& object : scene.world.Objects()) {
    (void)object->BoundsOf();
  }
  const auto render_row = [&](const int row) -> bool {
    for (int column = 0; column < width; ++column) {
      if (cancel != nullptr && cancel->load()) {
        return false;
      }
      auto& output = image.pixels[static_cast<std::size_t>(row) * static_cast<std::size_t>(width) +
                                  static_cast<std::size_t>(column)];
      if (!preview) {
        auto ray = camera.GetRayForPixel(column, row);
        output = ray_tracer::ColorAt(scene.world, ray);
        continue;
      }

      const int source_x = PreviewSamplePixel(column, width, full_width);
      const int source_y = PreviewSamplePixel(row, height, full_height);
      auto ray = camera.GetRayForPixel(source_x, source_y);
      output = ray_tracer::ColorAt(scene.world, ray);
    }
    return true;
  };
  if (!ParallelRenderRows(height, render_row, cancel, progress)) {
    return {};
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
