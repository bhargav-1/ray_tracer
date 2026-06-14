#include "app/scene_editor/scene_panel.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <string>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/imgui_file_dialog.h"
#include "app/scene_editor/widget_helpers.h"
#include "app/scene_editor/object_panel.h"
#include "app/scene_editor/render_controller.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"

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

  DrawFilePanel(app, render_state, path_buffer);
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
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::SmallButton(editor_state.selection.target == SelectionTarget::kCamera ? "Selected" : "Select")) {
      editor_state.selection = Selection{SelectionTarget::kCamera, 0};
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

  if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
    int delete_light_index = -1;
    for (std::size_t i = 0; i < scene.lights.size(); ++i) {
      ImGui::PushID(static_cast<int>(i));
      const std::string label = "Light " + std::to_string(i + 1);
      if (ImGui::SmallButton(editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index == i
                                 ? "Selected"
                                 : "Select")) {
        editor_state.selection = Selection{SelectionTarget::kLight, i};
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("Remove")) {
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
      editor_state.selection = Selection{SelectionTarget::kLight, scene.lights.size() - 1};
      changed = true;
    }
  }

  ImGui::Separator();
  ImGui::Text("Objects (%zu)", scene.objects.size());
  int delete_index = -1;
  for (std::size_t i = 0; i < scene.objects.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    const bool selected = editor_state.selection.target == SelectionTarget::kObject && editor_state.selection.index == i;
    if (ImGui::SmallButton(selected ? "Selected" : "Select")) {
      editor_state.selection = Selection{SelectionTarget::kObject, i};
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(scene.objects[i].visible ? "Hide" : "Show")) {
      scene.objects[i].visible = !scene.objects[i].visible;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Remove")) {
      delete_index = static_cast<int>(i);
    }
    ImGui::SameLine();
    const bool open = ImGui::CollapsingHeader(scene.objects[i].name.c_str());
    if (open) {
      changed |= DrawObjectEditor(scene.objects[i]);
    }
    ImGui::PopID();
  }

  if (delete_index >= 0 && delete_index < static_cast<int>(scene.objects.size())) {
    if (editor_state.selection.target == SelectionTarget::kObject) {
      const auto selected_index = static_cast<int>(editor_state.selection.index);
      if (selected_index == delete_index) {
        editor_state.selection = Selection{};
      } else if (selected_index > delete_index) {
        --editor_state.selection.index;
      }
    }
    scene.objects.erase(scene.objects.begin() + delete_index);
    changed = true;
  }

  return changed;
}

}  // namespace ray_tracer::scene_editor
