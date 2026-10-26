#include "app/scene_editor/render_controller.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <ratio>
#include <thread>
#include <utility>

#include "app/scene/scene_compiler.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/scene_renderer.h"

namespace ray_tracer::scene_editor {

auto StopRender(UiRenderState& render_state) -> void {
  render_state.cancel.store(true);
  if (render_state.worker.joinable()) {
    render_state.worker.join();
  }
  render_state.rendering.store(false);
  render_state.cancel.store(false);
}

auto StartRender(UiRenderState& render_state,
                 const scene_runtime::CompiledScene& compiled_scene,
                 const bool preview) -> void {
  StopRender(render_state);
  const int render_id = render_state.next_render_id++;
  {
    const std::lock_guard lock{render_state.image_mutex};
    render_state.pending_image = {};
    render_state.pending_render_id = 0;
    render_state.pending_is_preview = false;
    render_state.texture_dirty.store(false, std::memory_order_release);
  }
  render_state.cancel.store(false);
  render_state.progress.store(0);
  render_state.rendering.store(true);
  render_state.is_preview = preview;
  render_state.active_render_id = render_id;
  const auto started = std::chrono::steady_clock::now();
  render_state.render_started = started;
  render_state.worker = std::thread([&render_state, compiled_scene, preview, render_id, started]() {
    auto image = SceneRenderer::Render(compiled_scene, preview, &render_state.cancel, &render_state.progress);
    const auto elapsed_ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    if (!render_state.cancel.load() && !image.pixels.empty()) {
      const std::lock_guard lock{render_state.image_mutex};
      render_state.pending_image = std::move(image);
      render_state.pending_render_id = render_id;
      render_state.pending_is_preview = preview;
      if (!preview) {
        render_state.last_full_render_ms.store(elapsed_ms);
      }
      render_state.texture_dirty.store(true, std::memory_order_release);
    }
    render_state.rendering.store(false);
  });
}

auto ScheduleRender(SceneEditorApp& app,
                    UiRenderState& render_state,
                    const bool preview,
                    const bool camera_only) -> void {
  const bool prepared = camera_only ? app.UpdateCamera() : app.RecompileCurrentScene();
  if (!prepared) {
    render_state.preview_pending = false;
    render_state.scene_rebuild_pending = false;
    render_state.high_res_pending = false;
    return;
  }
  const auto& optional_compiled_scene = app.CompiledScene();
  if (!optional_compiled_scene.has_value()) {
    return;
  }
  const auto& compiled_scene = optional_compiled_scene.value();
  app.ClearRenderRequest();
  render_state.preview_pending = false;
  render_state.scene_rebuild_pending = false;
  render_state.high_res_pending = preview;
  StartRender(render_state, compiled_scene, preview);
}

auto UploadPendingTexture(UiRenderState& render_state) -> void {
  if (render_state.rendering.load() || !render_state.texture_dirty.load(std::memory_order_acquire)) {
    return;
  }
  RenderPixels image;
  int render_id = 0;
  bool is_preview = false;
  {
    const std::lock_guard lock{render_state.image_mutex};
    image = std::move(render_state.pending_image);
    render_id = render_state.pending_render_id;
    is_preview = render_state.pending_is_preview;
    render_state.pending_render_id = 0;
    render_state.pending_is_preview = false;
    render_state.texture_dirty.store(false, std::memory_order_release);
  }
  if (image.pixels.empty()) {
    return;
  }
  const auto rgb = SceneRenderer::ToRgb8(image);
  render_state.texture.UploadRgb(image.width, image.height, rgb);
  render_state.displayed_width = image.width;
  render_state.displayed_height = image.height;
  render_state.displayed_render_id = render_id;
  render_state.displayed_is_preview = is_preview;
}

auto RequestPreviewRender(UiRenderState& render_state, const bool camera_only) -> void {
  render_state.last_interaction = std::chrono::steady_clock::now();
  render_state.high_res_pending = true;
  render_state.preview_pending = true;
  render_state.scene_rebuild_pending |= !camera_only;
  // Keep completed previews flowing while dragging, but interrupt a full render.
  if (render_state.rendering.load() && !render_state.is_preview) {
    render_state.cancel.store(true);
  }
}

auto ProcessDeferredRenders(SceneEditorApp& app, UiRenderState& render_state) -> void {
  if (app.State().render_state.render_requested) {
    if (!render_state.preview_pending) {
      render_state.last_interaction = std::chrono::steady_clock::now();
    }
    render_state.preview_pending = true;
    render_state.scene_rebuild_pending = true;
  }

  // Upload the previous frame before starting another, so it cannot be discarded.
  if (render_state.rendering.load() || render_state.texture_dirty.load(std::memory_order_acquire)) {
    return;
  }
  if (render_state.preview_pending) {
    ScheduleRender(app, render_state, true, !render_state.scene_rebuild_pending);
    return;
  }

  if (render_state.high_res_pending && !render_state.interaction_active) {
    const auto elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - render_state.last_interaction).count();
    if (elapsed >= static_cast<double>(render_state.high_res_delay)) {
      render_state.high_res_pending = false;
      const auto& compiled_scene = app.CompiledScene();
      if (compiled_scene.has_value()) {
        StartRender(render_state, *compiled_scene, false);
      }
    }
  }
}

}  // namespace ray_tracer::scene_editor
