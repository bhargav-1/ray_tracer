#pragma once

#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/render_state.h"

namespace ray_tracer::scene_editor {

auto StopRender(UiRenderState& render_state) -> void;
auto StartRender(UiRenderState& render_state,
                 const scene_runtime::CompiledScene& compiled_scene,
                 bool preview) -> void;
auto ScheduleRender(SceneEditorApp& app, UiRenderState& render_state, bool preview) -> void;
auto UploadPendingTexture(UiRenderState& render_state) -> void;
auto RequestPreviewRender(SceneEditorApp& app, UiRenderState& render_state) -> void;
auto ProcessDeferredRenders(SceneEditorApp& app, UiRenderState& render_state) -> void;

}  // namespace ray_tracer::scene_editor
