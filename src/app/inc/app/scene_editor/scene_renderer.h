#pragma once

#include <atomic>
#include <vector>

#include "app/color.h"
#include "app/scene/scene_compiler.h"

namespace ray_tracer::scene_editor {

struct RenderPixels {
  int width{0};
  int height{0};
  std::vector<ray_tracer::Color> pixels;
};

class SceneRenderer {
 public:
  static constexpr int kPreviewDivisor = 8;

  [[nodiscard]] auto Render(const scene_runtime::CompiledScene& scene,
                            bool preview,
                            std::atomic<bool>* cancel,
                            std::atomic<int>* progress) const -> RenderPixels;

  [[nodiscard]] static auto ToRgb8(const RenderPixels& image) -> std::vector<unsigned char>;
};

}  // namespace ray_tracer::scene_editor
