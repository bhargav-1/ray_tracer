#include "app/scene_editor/add_panel.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene/shape_catalog.h"
#include "app/scene_editor/imgui_file_dialog.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"

namespace ray_tracer::scene_editor {
namespace {

auto FindObject(std::vector<scene::Object>& objects, const std::vector<std::size_t>& path) -> scene::Object* {
  auto* current_objects = &objects;
  scene::Object* object = nullptr;
  for (std::size_t depth = 0; depth < path.size(); ++depth) {
    const auto index = path[depth];
    if (index >= current_objects->size()) {
      return nullptr;
    }
    object = &(*current_objects)[index];
    if (depth + 1 < path.size()) {
      if (!object->IsContainer()) {
        return nullptr;
      }
      current_objects = &object->Children();
    }
  }
  return object;
}

}  // namespace

auto DrawAddPanel(SceneEditorApp& app, UiRenderState& render_state) -> bool {
  auto& scene = app.MutableScene();
  auto& state = app.MutableState();
  auto* destination = &scene.objects;
  std::vector<std::size_t> destination_path;
  std::string destination_name = "Scene Root";
  if (state.selection.target == SelectionTarget::kObject) {
    if (auto* selected = FindObject(scene.objects, state.selection.object_path);
        selected != nullptr && selected->IsGroup()) {
      destination = &selected->Group().children;
      destination_path = state.selection.object_path;
      destination_name = selected->name;
    }
  }

  bool changed = false;
  ImGui::TextWrapped("Add to: %s", destination_name.c_str());
  for (const auto& registration : shape_catalog::All()) {
    const auto display_name = shape_catalog::DisplayName(registration.serialized_name);
    const auto button_label = "+ " + display_name;
    if (ImGui::Button(button_label.c_str(), ImVec2(-1.0F, 40.0F))) {
      if (registration.type == scene::ObjectType::kObj) {
        state.obj_import_parent_path = destination_path;
        ImGuiFileDialog::Instance()->OpenDialog(
            "AddObjDialog", "Add OBJ Model", ".obj", std::filesystem::path{RAY_TRACER_ASSET_DIR}, "Add");
        continue;
      }
      scene::Object object;
      object.name = display_name + " " + std::to_string(destination->size() + 1);
      object.Shape().type = registration.type;
      if (registration.type == scene::ObjectType::kSphere || registration.type == scene::ObjectType::kCube ||
          registration.supports_axial_bounds) {
        object.transform.translate.y = 1.0F;
      }
      destination->push_back(std::move(object));
      auto path = destination_path;
      path.push_back(destination->size() - 1);
      state.selection = Selection::ForObject(std::move(path));
      changed = true;
    }
  }
  if (ImGuiFileDialog::Instance()->Display("AddObjDialog") && ImGuiFileDialog::Instance()->IsOk()) {
    if (app.AddObj(ImGuiFileDialog::Instance()->GetFilePathName(), state.obj_import_parent_path)) {
      return true;  // The scene was replaced; recompute destination pointers on the next frame.
    }
  }
  if (ImGui::Button("+ CSG", ImVec2(-1.0F, 40.0F))) {
    auto object = scene::MakeCsg();
    object.name += " " + std::to_string(destination->size() + 1);
    object.transform.translate.y = 1.0F;
    destination->push_back(std::move(object));
    auto path = destination_path;
    path.push_back(destination->size() - 1);
    state.selection = Selection::ForObject(std::move(path));
    changed = true;
  }
  if (ImGui::Button("+ Group", ImVec2(-1.0F, 40.0F))) {
    scene::Object group;
    group.name = "Group " + std::to_string(destination->size() + 1);
    group.content = scene::GroupSpec{};
    destination->push_back(std::move(group));
    auto path = destination_path;
    path.push_back(destination->size() - 1);
    state.selection = Selection::ForObject(std::move(path));
    changed = true;
  }
  ImGui::Separator();
  ImGui::TextWrapped("Preview idle delay");
  ImGui::SliderFloat("##full_res_idle_delay", &render_state.high_res_delay, 0.0F, 10.0F, "%.1fs");
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip(
        "Extra wait after interaction stops. 0 starts the full render as soon as the latest preview finishes.");
  }
  return changed;
}

}  // namespace ray_tracer::scene_editor
