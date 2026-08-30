#include "app/scene_editor/add_panel.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/shape_catalog.h"

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
      if (!object->IsGroup()) {
        return nullptr;
      }
      current_objects = &object->Group().children;
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
  ImGui::SliderFloat("Full-res delay", &render_state.high_res_delay, 0.5F, 10.0F, "%.1fs");
  return changed;
}

}  // namespace ray_tracer::scene_editor
