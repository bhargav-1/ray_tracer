#pragma once

#include <array>
#include <filesystem>

#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/render_state.h"

namespace ray_tracer::scene_editor {

auto CopyPathToBuffer(const std::filesystem::path& path, std::array<char, 1024>& buffer) -> void;
auto DrawScenePanel(SceneEditorApp& app, UiRenderState& render_state, std::array<char, 1024>& path_buffer) -> bool;

}  // namespace ray_tracer::scene_editor
