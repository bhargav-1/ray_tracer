#include "app/scene_editor/add_panel.h"

#include <string>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"

namespace ray_tracer::scene_editor {

auto DrawAddPanel(SceneEditorApp& app, UiRenderState& render_state) -> bool {
  auto& scene = app.MutableScene();
  auto& state = app.MutableState();
  bool changed = false;
  if (ImGui::Button("+ Sphere", ImVec2(-1.0F, 40.0F))) {
    scene::Object object;
    object.name = "Sphere " + std::to_string(scene.objects.size() + 1);
    object.type = scene::ObjectType::kSphere;
    object.transform.translate.y = 1.0F;
    scene.objects.push_back(object);
    state.selection = Selection{SelectionTarget::kObject, scene.objects.size() - 1};
    changed = true;
  }
  if (ImGui::Button("+ Plane", ImVec2(-1.0F, 40.0F))) {
    scene::Object object;
    object.name = "Plane " + std::to_string(scene.objects.size() + 1);
    object.type = scene::ObjectType::kPlane;
    scene.objects.push_back(object);
    state.selection = Selection{SelectionTarget::kObject, scene.objects.size() - 1};
    changed = true;
  }
  ImGui::Separator();
  ImGui::SliderFloat("Full-res delay", &render_state.high_res_delay, 0.5F, 10.0F, "%.1fs");
  return changed;
}

}  // namespace ray_tracer::scene_editor
