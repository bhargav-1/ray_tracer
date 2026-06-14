#include "app/scene_editor/widget_helpers.h"

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"

namespace ray_tracer::scene_editor {

auto SliderF(const char* label, float* value, float low, float high) -> bool {
  ImGui::SliderFloat(label, value, low, high);
  return ImGui::IsItemDeactivatedAfterEdit();
}

auto DragF3(const char* label, scene::Vec3& value, float speed, float low, float high) -> bool {
  ImGui::DragFloat3(label, &value.x, speed, low, high);
  return ImGui::IsItemDeactivatedAfterEdit();
}

auto ColorE3(const char* label, scene::ColorRgb& value) -> bool {
  ImGui::ColorEdit3(label, &value.r);
  return ImGui::IsItemDeactivatedAfterEdit();
}

}  // namespace ray_tracer::scene_editor
