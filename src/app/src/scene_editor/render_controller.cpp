#include "app/scene_editor/render_controller.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include "app/scene/scene_compiler.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/scene_debug_log.h"
#include "app/scene_editor/scene_renderer.h"

namespace ray_tracer::scene_editor {

auto StopRender(UiRenderState& render_state) -> void {
  render_state.cancel.store(true);
  if (render_state.worker.joinable()) {
    std::cerr << "[scene_editor] cancel render id=" << render_state.active_render_id << '\n';
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
  const int preview_divisor = preview ? SceneRenderer::kPreviewDivisor : 1;
  std::cerr << "[scene_editor] begin render id=" << render_id << " mode=" << (preview ? "preview" : "full")
            << " compiled=" << compiled_scene.camera.GetHorizontalSize() << 'x'
            << compiled_scene.camera.GetVerticalSize()
            << " objects=" << compiled_scene.world.Objects().size()
            << " lights=" << compiled_scene.world.Lights().size()
            << " preview_divisor=" << preview_divisor << '\n';
  LogRayFingerprint(preview ? "begin-preview" : "begin-full", render_id, compiled_scene.camera);
  LogRuntimeSceneSnapshot(preview ? "preview" : "full", render_id, compiled_scene);
  render_state.worker = std::thread([&render_state, compiled_scene, preview, render_id]() {
    const auto start = std::chrono::steady_clock::now();
    auto image = SceneRenderer::Render(compiled_scene, preview, &render_state.cancel, &render_state.progress);
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    if (!render_state.cancel.load() && !image.pixels.empty()) {
      const std::lock_guard lock{render_state.image_mutex};
      render_state.pending_image = std::move(image);
      render_state.pending_render_id = render_id;
      render_state.pending_is_preview = preview;
      render_state.texture_dirty.store(true, std::memory_order_release);
      std::cerr << "[scene_editor] finish render id=" << render_id << " mode=" << (preview ? "preview" : "full")
                << " image=" << render_state.pending_image.width << 'x' << render_state.pending_image.height
                << " elapsed_ms=" << elapsed_ms << '\n';
    } else {
      std::cerr << "[scene_editor] discard render id=" << render_id << " mode=" << (preview ? "preview" : "full")
                << " cancelled=" << (render_state.cancel.load() ? 1 : 0)
                << " elapsed_ms=" << elapsed_ms << '\n';
    }
    render_state.rendering.store(false);
  });
}

auto ScheduleRender(SceneEditorApp& app, UiRenderState& render_state, const bool preview) -> void {
  if (!app.RecompileCurrentScene()) {
    return;
  }
  const auto& optional_compiled_scene = app.CompiledScene();
  if (!optional_compiled_scene.has_value()) {
    return;
  }
  const auto& compiled_scene = optional_compiled_scene.value();
  app.ClearRenderRequest();
  if (preview) {
    render_state.last_interaction = std::chrono::steady_clock::now();
    render_state.high_res_pending = true;
  } else {
    render_state.high_res_pending = false;
  }
  const auto& scene = app.CurrentScene();
  LogAuthoringSceneSnapshot(preview ? "preview" : "full", scene);
  if (scene.camera.has_value()) {
    const auto& camera = *scene.camera;
    std::cerr << std::fixed << std::setprecision(3)
              << "[scene_editor] schedule mode=" << (preview ? "preview" : "full")
              << " camera_from=[" << camera.from.x << ' ' << camera.from.y << ' ' << camera.from.z << ']'
              << " camera_to=[" << camera.to.x << ' ' << camera.to.y << ' ' << camera.to.z << ']'
              << " fov=" << camera.fov_deg
              << " res=" << camera.resolution.width << 'x' << camera.resolution.height
              << " scene_objects=" << scene.objects.size()
              << " visible=" << CountVisibleObjects(scene)
              << " compiled_objects=" << compiled_scene.world.Objects().size()
              << " scene_hash=" << std::hex << std::setw(16) << std::setfill('0') << SceneFingerprint(scene)
              << std::dec << std::setfill(' ') << '\n';
  }
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
  std::cerr << "[scene_editor] upload render id=" << render_id
            << " image=" << image.width << 'x' << image.height
            << " aspect=" << std::fixed << std::setprecision(6)
            << static_cast<double>(image.width) / static_cast<double>(image.height) << '\n';
}

auto RequestPreviewRender(SceneEditorApp& app, UiRenderState& render_state) -> void {
  render_state.last_interaction = std::chrono::steady_clock::now();
  render_state.high_res_pending = true;
  ScheduleRender(app, render_state, true);
}

auto ProcessDeferredRenders(SceneEditorApp& app, UiRenderState& render_state) -> void {
  if (!render_state.rendering.load() && app.State().render_state.render_requested) {
    ScheduleRender(app, render_state, true);
  }

  if (render_state.high_res_pending && !render_state.rendering.load()) {
    const auto elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - render_state.last_interaction).count();
    if (elapsed >= static_cast<double>(render_state.high_res_delay)) {
      ScheduleRender(app, render_state, false);
    }
  }
}

}  // namespace ray_tracer::scene_editor
