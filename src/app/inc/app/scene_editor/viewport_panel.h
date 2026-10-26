#pragma once

#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"

namespace ray_tracer::scene_editor {

struct ViewportPanelResult {
  bool changed{false};
  bool camera_moved{false};
};

auto DrawViewportPanel(SceneEditorApp& app, UiRenderState& render_state) -> ViewportPanelResult;

}  // namespace ray_tracer::scene_editor
