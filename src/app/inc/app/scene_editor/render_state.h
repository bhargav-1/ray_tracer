#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>

#include "app/scene_editor/gl_texture.h"
#include "app/scene_editor/scene_renderer.h"

namespace ray_tracer::scene_editor {

enum class GizmoAxis : std::uint8_t { kNone, kX, kY, kZ };

struct UiRenderState {
  SceneRenderer renderer;
  GlTexture texture;
  std::atomic<bool> rendering{false};
  std::atomic<bool> cancel{false};
  std::atomic<int> progress{0};
  std::thread worker;
  std::mutex image_mutex;
  RenderPixels pending_image;
  std::atomic<bool> texture_dirty{false};
  int next_render_id{1};
  int active_render_id{0};
  int pending_render_id{0};
  bool pending_is_preview{false};
  int displayed_width{0};
  int displayed_height{0};
  int displayed_render_id{0};
  bool displayed_is_preview{false};
  bool is_preview{false};
  bool high_res_pending{false};
  bool preview_pending{false};
  bool scene_rebuild_pending{false};
  bool interaction_active{false};
  float high_res_delay{0.0F};
  std::chrono::steady_clock::time_point last_interaction;
  std::chrono::steady_clock::time_point render_started;
  std::atomic<double> last_full_render_ms{-1.0};
  GizmoAxis active_axis{GizmoAxis::kNone};
};

}  // namespace ray_tracer::scene_editor
