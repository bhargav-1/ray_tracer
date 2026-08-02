#pragma once

#include <string>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/render_state.h"

namespace ray_tracer::scene_editor {

struct OverlayResult {
  bool changed{false};
  bool block_camera{false};
};

auto ObjectTypeLabel(scene::ObjectType type) -> std::string;
auto SelectedName(const scene::Scene& scene, const Selection& selection) -> std::string;
auto DrawViewportOverlay(scene::Scene& scene,
                         EditorState& editor_state,
                         UiRenderState& render_state,
                         ImVec2 image_min,
                         ImVec2 image_size) -> OverlayResult;

}  // namespace ray_tracer::scene_editor
