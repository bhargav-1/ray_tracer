#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/materials/color.h"
#include "app/scene/scene.h"
#include "app/scene/scene_compiler.h"

namespace ray_tracer::scene_editor {

enum class SelectionTarget : std::uint8_t { kNone, kObject, kLight, kCamera };
enum class GizmoMode : std::uint8_t { kTranslate, kRotate, kScale };

struct Selection {
  SelectionTarget target{SelectionTarget::kNone};
  std::size_t index{0};
  std::vector<std::size_t> object_path;

  [[nodiscard]] static auto ForObject(std::vector<std::size_t> path) -> Selection {
    return Selection{.target = SelectionTarget::kObject, .index = 0, .object_path = std::move(path)};
  }
  [[nodiscard]] static auto ForLight(std::size_t light_index) -> Selection {
    return Selection{.target = SelectionTarget::kLight, .index = light_index, .object_path = {}};
  }
  [[nodiscard]] static auto ForCamera() -> Selection {
    return Selection{.target = SelectionTarget::kCamera, .index = 0, .object_path = {}};
  }
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
  GizmoMode gizmo_mode{GizmoMode::kTranslate};
  RenderState render_state;
  bool show_selected_viewport{false};
  std::string status_message;
  std::vector<std::size_t> obj_import_parent_path;
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
  auto AddObj(const std::filesystem::path& path, const std::vector<std::size_t>& parent_path = {}) -> bool;
  auto SetCurrentScene(scene::Scene scene) -> bool;
  auto RecompileCurrentScene() -> bool;
  auto UpdateCamera() -> bool;
  auto ClearRenderRequest() -> void { state_.render_state.render_requested = false; }

 private:
  auto CompileCurrentScene() -> bool;

  scene::Scene current_scene_;
  EditorState state_;
  std::optional<scene_runtime::CompiledScene> compiled_scene_;
};

}  // namespace ray_tracer::scene_editor
