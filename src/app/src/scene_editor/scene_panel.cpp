#include "app/scene_editor/scene_panel.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/imgui_file_dialog.h"
#include "app/scene_editor/widget_helpers.h"
#include "app/scene_editor/object_panel.h"
#include "app/scene_editor/render_controller.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/shape_catalog.h"

namespace ray_tracer::scene_editor {
namespace {

auto DrawFilePanel(SceneEditorApp& app, UiRenderState& render_state, std::array<char, 1024>& path_buffer) -> void {
  ImGui::InputText("Path", path_buffer.data(), path_buffer.size());
  if (ImGui::Button("Load")) {
    const auto start_path = std::filesystem::path{path_buffer.data()}.empty()
                                ? std::filesystem::path{RAY_TRACER_ASSET_DIR} / "scenes"
                                : std::filesystem::path{path_buffer.data()};
    ImGuiFileDialog::Instance()->OpenDialog("LoadSceneDialog", "Load Scene", ".yaml,.yml", start_path);
  }
  ImGui::SameLine();
  if (ImGui::Button("Save As")) {
    const auto start_path = std::filesystem::path{path_buffer.data()}.empty()
                                ? std::filesystem::path{RAY_TRACER_ASSET_DIR} / "scenes" / "scene.yaml"
                                : std::filesystem::path{path_buffer.data()};
    ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog", "Save Scene As", ".yaml,.yml", start_path, "Save");
  }
  if (ImGuiFileDialog::Instance()->Display("LoadSceneDialog")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      const auto selected_path = ImGuiFileDialog::Instance()->GetFilePathName();
      if (app.LoadScene(selected_path)) {
        CopyPathToBuffer(app.State().current_path, path_buffer);
        ScheduleRender(app, render_state, false);
      }
    }
  }
  if (ImGuiFileDialog::Instance()->Display("SaveSceneDialog")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      const auto selected_path = ImGuiFileDialog::Instance()->GetFilePathName();
      if (app.SaveScene(selected_path)) {
        CopyPathToBuffer(app.State().current_path, path_buffer);
      }
    }
  }
  if (!app.State().status_message.empty()) {
    ImGui::TextWrapped("%s", app.State().status_message.c_str());
  }
}

using ObjectPath = std::vector<std::size_t>;

auto CountObjects(const std::vector<scene::Object>& objects) -> std::size_t {  // NOLINT(misc-no-recursion)
  std::size_t count = objects.size();
  for (const auto& object : objects) {
    if (object.IsGroup()) {
      count += CountObjects(object.Group().children);
    }
  }
  return count;
}

auto FindObject(std::vector<scene::Object>& objects, const ObjectPath& path) -> scene::Object* {
  auto* current_objects = &objects;
  scene::Object* object = nullptr;
  for (std::size_t depth = 0; depth < path.size(); ++depth) {
    const auto index = path[depth];
    if (index >= current_objects->size()) {
      return nullptr;
    }
    object = &(*current_objects)[index];
    if (depth + 1 < path.size()) {
      if (!object->IsGroup()) {
        return nullptr;
      }
      current_objects = &object->Group().children;
    }
  }
  return object;
}

auto IsSelected(const Selection& selection, const ObjectPath& path) -> bool {
  return selection.target == SelectionTarget::kObject && selection.object_path == path;
}

auto AdjustSelectionAfterErase(Selection& selection, const ObjectPath& parent_path, std::size_t erased_index) -> void {
  if (selection.target != SelectionTarget::kObject || selection.object_path.size() <= parent_path.size() ||
      !std::equal(parent_path.begin(), parent_path.end(), selection.object_path.begin())) {
    return;
  }
  auto& selected_index = selection.object_path[parent_path.size()];
  if (selected_index == erased_index) {
    selection = Selection{};
  } else if (selected_index > erased_index) {
    --selected_index;
  }
}

auto AddObjectMenu(std::vector<scene::Object>& objects, const ObjectPath& parent_path, EditorState& state) -> bool {
  bool changed = false;
  for (const auto& registration : shape_catalog::All()) {
    if (registration.type == scene::ObjectType::kObj) {
      continue;  // OBJ imports use the file picker in the Add panel.
    }
    const auto display_name = shape_catalog::DisplayName(registration.serialized_name);
    if (ImGui::MenuItem(display_name.c_str())) {
      scene::Object object;
      object.name = display_name + " " + std::to_string(objects.size() + 1);
      object.Shape().type = registration.type;
      if (registration.type == scene::ObjectType::kSphere || registration.type == scene::ObjectType::kCube ||
          registration.supports_axial_bounds) {
        object.transform.translate.y = 1.0F;
      }
      objects.push_back(std::move(object));
      auto path = parent_path;
      path.push_back(objects.size() - 1);
      state.selection = Selection::ForObject(std::move(path));
      changed = true;
    }
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Group")) {
    scene::Object group;
    group.name = "Group " + std::to_string(objects.size() + 1);
    group.content = scene::GroupSpec{};
    objects.push_back(std::move(group));
    auto path = parent_path;
    path.push_back(objects.size() - 1);
    state.selection = Selection::ForObject(std::move(path));
    changed = true;
  }
  return changed;
}

auto DrawObjectTree(std::vector<scene::Object>& objects,  // NOLINT(misc-no-recursion)
                    const ObjectPath& parent_path,
                    EditorState& state) -> bool {
  bool changed = false;
  std::optional<std::size_t> delete_index;
  for (std::size_t index = 0; index < objects.size(); ++index) {
    auto path = parent_path;
    path.push_back(index);
    auto& object = objects[index];
    ImGui::PushID(static_cast<int>(index));
    if (ImGui::SmallButton(IsSelected(state.selection, path) ? "Selected###select" : "Select###select")) {
      state.selection = Selection::ForObject(path);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(object.visible ? "Hide###visibility" : "Show###visibility")) {
      object.visible = !object.visible;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Remove###remove")) {
      delete_index = index;
    }
    if (object.IsGroup()) {
      ImGui::SameLine();
      if (ImGui::SmallButton("+ Child")) {
        ImGui::OpenPopup("AddChild");
      }
      if (ImGui::BeginPopup("AddChild")) {
        changed |= AddObjectMenu(object.Group().children, path, state);
        ImGui::EndPopup();
      }
    }
    ImGui::SameLine();
    const auto flags = object.IsGroup() ? ImGuiTreeNodeFlags_DefaultOpen
                                        : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    const auto node_label = object.name + "###object_node";
    const bool open = ImGui::TreeNodeEx(node_label.c_str(), flags);
    if (object.IsGroup() && open) {
      changed |= DrawObjectTree(object.Group().children, path, state);
      ImGui::TreePop();
    }
    ImGui::PopID();
  }
  if (delete_index.has_value()) {
    AdjustSelectionAfterErase(state.selection, parent_path, *delete_index);
    objects.erase(objects.begin() + static_cast<std::ptrdiff_t>(*delete_index));
    changed = true;
  }
  return changed;
}

}  // namespace

auto CopyPathToBuffer(const std::filesystem::path& path, std::array<char, 1024>& buffer) -> void {
  const auto text = path.string();
  buffer.fill('\0');
  const auto count = std::min(text.size(), buffer.size() - 1);
  std::copy_n(text.data(), count, buffer.data());
}

auto DrawScenePanel(SceneEditorApp& app, UiRenderState& render_state, std::array<char, 1024>& path_buffer) -> bool {
  bool changed = false;
  auto& scene = app.MutableScene();
  auto& editor_state = app.MutableState();

  ImGui::PushID("File");
  DrawFilePanel(app, render_state, path_buffer);
  ImGui::PopID();
  ImGui::Separator();
  if (render_state.rendering.load()) {
    ImGui::TextColored(ImVec4(1.0F, 1.0F, 0.0F, 1.0F), "Rendering... %d%%%s",
                       render_state.progress.load(), render_state.is_preview ? " (preview)" : "");
  } else if (ImGui::Button("Render", ImVec2(-1.0F, 30.0F))) {
    ScheduleRender(app, render_state, false);
  }

  if (!scene.camera.has_value()) {
    scene.camera = scene::Camera{};
    changed = true;
  }
  ImGui::PushID("Camera");
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::SmallButton(editor_state.selection.target == SelectionTarget::kCamera ? "Selected###select" : "Select###select")) {
      editor_state.selection = Selection::ForCamera();
    }
    changed |= DragF3("From", scene.camera->from, 0.1F, -50.0F, 50.0F);
    changed |= DragF3("Look At", scene.camera->to, 0.1F, -50.0F, 50.0F);
    changed |= DragF3("Up", scene.camera->up, 0.01F, -1.0F, 1.0F);
    changed |= SliderF("FOV (deg)", &scene.camera->fov_deg, 10.0F, 170.0F);
    ImGui::InputInt("Width", &scene.camera->resolution.width, 100);
    changed |= ImGui::IsItemDeactivatedAfterEdit();
    ImGui::InputInt("Height", &scene.camera->resolution.height, 100);
    changed |= ImGui::IsItemDeactivatedAfterEdit();
    scene.camera->resolution.width = std::clamp(scene.camera->resolution.width, 100, 4000);
    scene.camera->resolution.height = std::clamp(scene.camera->resolution.height, 100, 4000);
  }

  ImGui::PopID();
  ImGui::PushID("Lights");
  if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
    int delete_light_index = -1;
    for (std::size_t i = 0; i < scene.lights.size(); ++i) {
      ImGui::PushID(static_cast<int>(i));
      const std::string label = "Light " + std::to_string(i + 1);
      if (ImGui::SmallButton(editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index == i
                                 ? "Selected###select"
                                 : "Select###select")) {
        editor_state.selection = Selection::ForLight(i);
      }
      ImGui::SameLine();
      if (ImGui::SmallButton(scene.lights[i].visible ? "Hide###visibility" : "Show###visibility")) {
        scene.lights[i].visible = !scene.lights[i].visible;
        changed = true;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("Remove###remove")) {
        delete_light_index = static_cast<int>(i);
      }
      ImGui::SameLine();
      if (ImGui::TreeNodeEx(label.c_str())) {
        changed |= DragF3("Position", scene.lights[i].position, 0.5F, -100.0F, 100.0F);
        changed |= ColorE3("Intensity", scene.lights[i].color);
        ImGui::TreePop();
      }
      ImGui::PopID();
    }
    if (delete_light_index >= 0 && delete_light_index < static_cast<int>(scene.lights.size())) {
      if (editor_state.selection.target == SelectionTarget::kLight) {
        const auto selected_index = static_cast<int>(editor_state.selection.index);
        if (selected_index == delete_light_index) {
          editor_state.selection = Selection{};
        } else if (selected_index > delete_light_index) {
          --editor_state.selection.index;
        }
      }
      scene.lights.erase(scene.lights.begin() + delete_light_index);
      changed = true;
    }
    if (ImGui::Button("+ Light")) {
      scene.lights.push_back(scene::Light{});
      editor_state.selection = Selection::ForLight(scene.lights.size() - 1);
      changed = true;
    }
  }
  ImGui::PopID();

  ImGui::Separator();
  ImGui::Text("Objects (%zu)", CountObjects(scene.objects));
  ImGui::PushID("Objects");
  changed |= DrawObjectTree(scene.objects, {}, editor_state);
  ImGui::PopID();
  if (editor_state.selection.target == SelectionTarget::kObject) {
    if (auto* selected = FindObject(scene.objects, editor_state.selection.object_path)) {
      ImGui::PushID("Inspector");
      for (const auto index : editor_state.selection.object_path) {
        ImGui::PushID(static_cast<int>(index));
      }
      ImGui::Separator();
      ImGui::Text("Inspector: %s", selected->name.c_str());
      changed |= DrawObjectEditor(*selected);
      for (std::size_t depth = 0; depth < editor_state.selection.object_path.size(); ++depth) {
        ImGui::PopID();
      }
      ImGui::PopID();
    } else {
      editor_state.selection = Selection{};
    }
  }

  return changed;
}

}  // namespace ray_tracer::scene_editor
