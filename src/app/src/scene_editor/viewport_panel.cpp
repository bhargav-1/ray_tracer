#include "app/scene_editor/viewport_panel.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ratio>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/camera_controls.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/viewport_overlay.h"

namespace ray_tracer::scene_editor {
namespace {

auto DrawTopLegend(SceneEditorApp& app) -> bool {
  auto& state = app.MutableState();
  ImGui::Text("Selected: %s", SelectedName(app.CurrentScene(), state.selection).c_str());
  ImGui::SameLine();
  ImGui::Checkbox("Show selected viewport", &state.show_selected_viewport);
  ImGui::SameLine();
  const bool camera_reset = ImGui::Button("Reset Camera") && app.ResetCamera();
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.92F, 0.25F, 0.22F, 1.0F), "X");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.30F, 0.85F, 0.32F, 1.0F), "Y");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.30F, 0.55F, 0.96F, 1.0F), "Z");

  if (ImGui::RadioButton("Move", state.gizmo_mode == GizmoMode::kTranslate)) {
    state.gizmo_mode = GizmoMode::kTranslate;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Scale", state.gizmo_mode == GizmoMode::kScale)) {
    state.gizmo_mode = GizmoMode::kScale;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Rotate", state.gizmo_mode == GizmoMode::kRotate)) {
    state.gizmo_mode = GizmoMode::kRotate;
  }
  return camera_reset;
}

}  // namespace

auto DrawViewportPanel(SceneEditorApp& app, UiRenderState& render_state) -> ViewportPanelResult {
  ViewportPanelResult result;
  constexpr auto kFixedPaneFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

  ImGui::Begin("Viewport", nullptr, kFixedPaneFlags);
  result.camera_moved = DrawTopLegend(app);
  ImGui::Separator();

  if (render_state.texture.Valid() && render_state.displayed_width > 0 && render_state.displayed_height > 0) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    // Reserve room below the image for render timing/status, even for square images.
    avail.y = std::max(1.0F, avail.y - 2.0F * ImGui::GetTextLineHeightWithSpacing());
    const auto image_width = static_cast<float>(render_state.displayed_width);
    const auto image_height = static_cast<float>(render_state.displayed_height);
    float scale = std::min(avail.x / image_width, avail.y / image_height);
    if (scale <= 0.0F) {
      scale = 1.0F;
    }
    const ImVec2 image_size{image_width * scale, image_height * scale};
    float cursor_x = ImGui::GetCursorPosX();
    float cursor_y = ImGui::GetCursorPosY();
    if (avail.x > image_size.x) {
      cursor_x += (avail.x - image_size.x) * 0.5F;
    }
    if (avail.y > image_size.y) {
      cursor_y += (avail.y - image_size.y) * 0.5F;
    }
    ImGui::SetCursorPos(ImVec2(cursor_x, cursor_y));
    // NOLINTNEXTLINE(performance-no-int-to-ptr): ImGui stores an opaque OpenGL handle.
    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(render_state.texture.Id())), image_size);
    const bool image_hovered = ImGui::IsItemHovered();
    const ImVec2 image_min = ImGui::GetItemRectMin();
    static bool left_drag_started_on_image = false;
    static bool right_drag_started_on_image = false;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      left_drag_started_on_image = image_hovered;
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      right_drag_started_on_image = image_hovered;
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      left_drag_started_on_image = false;
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      right_drag_started_on_image = false;
    }
    auto& scene = app.MutableScene();
    OverlayResult overlay;
    if (app.State().show_selected_viewport) {
      overlay = DrawViewportOverlay(scene, app.MutableState(), render_state, image_min, image_size, image_hovered);
    }
    result.changed |= overlay.changed;

    if (scene.camera.has_value() && image_hovered && !overlay.block_camera) {
      const ImVec2 drag = ImGui::GetIO().MouseDelta;
      if (left_drag_started_on_image && ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
          (drag.x != 0.0F || drag.y != 0.0F)) {
        OrbitCamera(*scene.camera, drag.x, drag.y);
        result.camera_moved = true;
      }
      if (right_drag_started_on_image && ImGui::IsMouseDragging(ImGuiMouseButton_Right) &&
          (drag.x != 0.0F || drag.y != 0.0F)) {
        PanCamera(*scene.camera, drag.x, drag.y);
        result.camera_moved = true;
      }
      const float scroll = ImGui::GetIO().MouseWheel;
      if (scroll != 0.0F) {
        ZoomCamera(*scene.camera, scroll);
        result.camera_moved = true;
      }
    }
  } else {
    ImGui::Text("No render yet");
  }

  if (render_state.rendering.load() && !render_state.is_preview) {
    const auto elapsed_ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - render_state.render_started)
            .count();
    ImGui::TextDisabled("Full render: %.1f ms elapsed", elapsed_ms);
  } else {
    if (render_state.high_res_pending) {
      ImGui::TextDisabled("Preview: full render starts after interaction stops");
    }
    const double full_render_ms = render_state.last_full_render_ms.load();
    if (full_render_ms >= 0.0) {
      ImGui::TextDisabled("Last completed full render: %.1f ms", full_render_ms);
    }
  }
  ImGui::End();

  return result;
}

}  // namespace ray_tracer::scene_editor
