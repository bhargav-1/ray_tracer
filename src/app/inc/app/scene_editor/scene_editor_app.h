#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "app/color.h"
#include "app/scene/scene.h"
#include "app/scene/scene_compiler.h"

namespace ray_tracer::scene_editor {

enum class SelectionTarget { None, Object, Light, Camera };
enum class GizmoMode { Translate, Rotate, Scale };

struct Selection {
  SelectionTarget target{SelectionTarget::None};
  std::size_t index{0};
};

struct RenderState {
  bool render_requested{false};
  bool rendering{false};
  bool is_preview{false};
  int progress{0};
  int width{0};
  int height{0};
  std::vector<ray_tracer::Color> pixels;
};

struct EditorState {
  std::filesystem::path current_path;
  Selection selection;
  GizmoMode gizmo_mode{GizmoMode::Translate};
  RenderState render_state;
  bool show_viewport_overlays{true};
  std::string status_message;
};

class SceneEditorApp {
 public:
  SceneEditorApp() = default;

  [[nodiscard]] auto CurrentScene() const -> const scene::Scene& { return current_scene_; }
  [[nodiscard]] auto MutableScene() -> scene::Scene& { return current_scene_; }
  [[nodiscard]] auto State() const -> const EditorState& { return state_; }
  [[nodiscard]] auto MutableState() -> EditorState& { return state_; }
  [[nodiscard]] auto CompiledScene() const -> const std::optional<scene_runtime::CompiledScene>& {
    return compiled_scene_;
  }

  auto LoadDefaultScene() -> bool;
  auto LoadScene(const std::filesystem::path& path) -> bool;
  auto SaveScene(const std::filesystem::path& path) -> bool;
  auto SetCurrentScene(scene::Scene scene) -> bool;
  auto RecompileCurrentScene() -> bool;
  auto ClearRenderRequest() -> void { state_.render_state.render_requested = false; }

 private:
  auto CompileCurrentScene() -> bool;

  scene::Scene current_scene_;
  EditorState state_;
  std::optional<scene_runtime::CompiledScene> compiled_scene_;
};

}  // namespace ray_tracer::scene_editor
