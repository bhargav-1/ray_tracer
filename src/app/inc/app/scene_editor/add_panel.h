#pragma once

#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/render_state.h"

namespace ray_tracer::scene_editor {

auto DrawAddPanel(SceneEditorApp& app, UiRenderState& render_state) -> bool;

}  // namespace ray_tracer::scene_editor
