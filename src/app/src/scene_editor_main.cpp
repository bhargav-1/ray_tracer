#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <numbers>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "app/scene_editor/gl_texture.h"
#include "app/scene_editor/imgui_file_dialog.h"
#include "app/scene_editor/scene_renderer.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/camera.h"
#include "app/material.h"
#include "app/matrix.h"
#include "app/plane.h"
#include "app/point.h"
#include "app/ray.h"
#include "app/scene/scene.h"
#include "app/shape.h"
#include "app/sphere.h"
#include "app/transformations.h"
#include "app/vec3.h"

namespace {

using ray_tracer::scene_editor::GizmoMode;
using ray_tracer::scene_editor::SceneEditorApp;
using ray_tracer::scene_editor::Selection;
using ray_tracer::scene_editor::SelectionTarget;

enum class GizmoAxis { None, X, Y, Z };

struct ProjectedPoint {
  ImVec2 screen;
  float depth{0.0F};
};

struct OverlayResult {
  bool changed{false};
  bool block_camera{false};
};

struct UiRenderState {
  ray_tracer::scene_editor::SceneRenderer renderer;
  ray_tracer::scene_editor::GlTexture texture;
  std::atomic<bool> rendering{false};
  std::atomic<bool> cancel{false};
  std::atomic<int> progress{0};
  std::thread worker;
  std::mutex image_mutex;
  ray_tracer::scene_editor::RenderPixels pending_image;
  std::atomic<bool> texture_dirty{false};
  int next_render_id{1};
  int active_render_id{0};
  int pending_render_id{0};
  bool pending_is_preview{false};
  int displayed_width{0};
  int displayed_height{0};
  int displayed_render_id{0};
  bool displayed_is_preview{false};
  bool is_preview{false};
  bool high_res_pending{false};
  float high_res_delay{2.0F};
  std::chrono::steady_clock::time_point last_interaction{};
  GizmoAxis active_axis{GizmoAxis::None};
};

auto GlfwErrorCallback(int error, const char* description) -> void {
  std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

auto ToPoint(const ray_tracer::scene::Vec3& value) -> ray_tracer::Point {
  return {value.x, value.y, value.z};
}

auto BuildTransformMatrix(const ray_tracer::scene::Transform& transform) -> ray_tracer::Matrix<4, 4> {
  auto matrix = ray_tracer::Matrix<4, 4>::Translation(
      transform.translate.x, transform.translate.y, transform.translate.z);
  constexpr auto kDegToRad = std::numbers::pi / 180.0;
  if (transform.rotate_deg.x != 0.0F) {
    matrix = matrix * ray_tracer::Matrix<4, 4>::RotationX(transform.rotate_deg.x * kDegToRad);
  }
  if (transform.rotate_deg.y != 0.0F) {
    matrix = matrix * ray_tracer::Matrix<4, 4>::RotationY(transform.rotate_deg.y * kDegToRad);
  }
  if (transform.rotate_deg.z != 0.0F) {
    matrix = matrix * ray_tracer::Matrix<4, 4>::RotationZ(transform.rotate_deg.z * kDegToRad);
  }
  return matrix * ray_tracer::Matrix<4, 4>::Scaling(transform.scale.x, transform.scale.y, transform.scale.z);
}

auto ObjectTypeLabel(ray_tracer::scene::ObjectType type) -> const char* {
  return type == ray_tracer::scene::ObjectType::Plane ? "Plane" : "Sphere";
}

auto SelectedName(const ray_tracer::scene::Scene& scene, const Selection& selection) -> std::string {
  if (selection.target == SelectionTarget::Camera) {
    return "Camera";
  }
  if (selection.target == SelectionTarget::Light && selection.index < scene.lights.size()) {
    return "Light " + std::to_string(selection.index + 1);
  }
  if (selection.target == SelectionTarget::Object && selection.index < scene.objects.size()) {
    return scene.objects[selection.index].name;
  }
  return "None";
}

auto SelectedObject(ray_tracer::scene::Scene& scene, const Selection& selection) -> ray_tracer::scene::Object* {
  if (selection.target != SelectionTarget::Object || selection.index >= scene.objects.size()) {
    return nullptr;
  }
  return &scene.objects[selection.index];
}

auto ShapeCenter(const ray_tracer::scene::Object& object) -> ray_tracer::Point {
  return ray_tracer::Point(BuildTransformMatrix(object.transform) * ray_tracer::Point(0, 0, 0));
}

auto DistanceSquared(ImVec2 lhs, ImVec2 rhs) -> float {
  const float dx = lhs.x - rhs.x;
  const float dy = lhs.y - rhs.y;
  return (dx * dx) + (dy * dy);
}

auto AxisName(GizmoAxis axis) -> const char* {
  switch (axis) {
    case GizmoAxis::X:
      return "X";
    case GizmoAxis::Y:
      return "Y";
    case GizmoAxis::Z:
      return "Z";
    case GizmoAxis::None:
      return "-";
  }
  return "-";
}

auto AxisIndex(GizmoAxis axis) -> int {
  switch (axis) {
    case GizmoAxis::X:
      return 0;
    case GizmoAxis::Y:
      return 1;
    case GizmoAxis::Z:
      return 2;
    case GizmoAxis::None:
      return -1;
  }
  return -1;
}

auto AxisColor(GizmoAxis axis) -> ImU32 {
  switch (axis) {
    case GizmoAxis::X:
      return IM_COL32(235, 65, 55, 255);
    case GizmoAxis::Y:
      return IM_COL32(80, 210, 85, 255);
    case GizmoAxis::Z:
      return IM_COL32(80, 140, 245, 255);
    case GizmoAxis::None:
      return IM_COL32(255, 255, 255, 255);
  }
  return IM_COL32(255, 255, 255, 255);
}

auto ProjectWorldPoint(const ray_tracer::scene::Camera& camera,
                       const ray_tracer::Point& world_point,
                       ImVec2 image_min,
                       ImVec2 image_size) -> std::optional<ProjectedPoint> {
  const auto view = ray_tracer::ViewTransform(ToPoint(camera.from), ToPoint(camera.to),
                                              ray_tracer::Vec3(camera.up.x, camera.up.y, camera.up.z));
  const auto camera_point = view * world_point;
  const double depth = -camera_point.GetZ();
  if (depth <= 0.01) {
    return std::nullopt;
  }

  const double half_view = std::tan((camera.fov_deg * std::numbers::pi / 180.0) / 2.0);
  const double aspect = static_cast<double>(image_size.x) / static_cast<double>(image_size.y);
  const double half_width = aspect >= 1.0 ? half_view : half_view * aspect;
  const double half_height = aspect >= 1.0 ? half_view / aspect : half_view;
  const double projected_x = camera_point.GetX() / depth;
  const double projected_y = camera_point.GetY() / depth;

  const float sx = image_min.x + static_cast<float>((half_width - projected_x) / (2.0 * half_width)) * image_size.x;
  const float sy = image_min.y + static_cast<float>((half_height - projected_y) / (2.0 * half_height)) * image_size.y;
  return ProjectedPoint{ImVec2(sx, sy), static_cast<float>(depth)};
}

auto AxisEndpoint(const ray_tracer::scene::Scene& scene,
                  const Selection& selection,
                  const ray_tracer::Point& center,
                  GizmoAxis axis) -> ray_tracer::Point {
  constexpr double kAxisLength = 1.25;
  if (selection.target == SelectionTarget::Object && selection.index < scene.objects.size()) {
    const auto transform = BuildTransformMatrix(scene.objects[selection.index].transform);
    switch (axis) {
      case GizmoAxis::X:
        return ray_tracer::Point(transform * ray_tracer::Point(kAxisLength, 0, 0));
      case GizmoAxis::Y:
        return ray_tracer::Point(transform * ray_tracer::Point(0, kAxisLength, 0));
      case GizmoAxis::Z:
        return ray_tracer::Point(transform * ray_tracer::Point(0, 0, kAxisLength));
      case GizmoAxis::None:
        return center;
    }
  }

  switch (axis) {
    case GizmoAxis::X:
      return {center.GetX() + kAxisLength, center.GetY(), center.GetZ()};
    case GizmoAxis::Y:
      return {center.GetX(), center.GetY() + kAxisLength, center.GetZ()};
    case GizmoAxis::Z:
      return {center.GetX(), center.GetY(), center.GetZ() + kAxisLength};
    case GizmoAxis::None:
      return center;
  }
  return center;
}

auto ApplyAxisDrag(ray_tracer::scene::Scene& scene,
                   ray_tracer::scene_editor::EditorState& editor_state,
                   GizmoAxis axis,
                   const ray_tracer::Point& center,
                   const ray_tracer::Point& endpoint,
                   ImVec2 center_screen,
                   ImVec2 endpoint_screen,
                   ImVec2 mouse_delta) -> bool {
  const int index = AxisIndex(axis);
  if (index < 0) {
    return false;
  }

  const float sx = endpoint_screen.x - center_screen.x;
  const float sy = endpoint_screen.y - center_screen.y;
  const float screen_len = std::sqrt((sx * sx) + (sy * sy));
  if (screen_len < 3.0F) {
    return false;
  }

  const double wx = endpoint.GetX() - center.GetX();
  const double wy = endpoint.GetY() - center.GetY();
  const double wz = endpoint.GetZ() - center.GetZ();
  const float world_len = static_cast<float>(std::sqrt((wx * wx) + (wy * wy) + (wz * wz)));
  const float signed_pixels = ((mouse_delta.x * sx) + (mouse_delta.y * sy)) / screen_len;
  const float world_delta = signed_pixels * std::max(world_len, 0.1F) / screen_len;

  if (editor_state.selection.target == SelectionTarget::Light && editor_state.selection.index < scene.lights.size()) {
    auto& position = scene.lights[editor_state.selection.index].position;
    (&position.x)[index] += world_delta;
    return true;
  }

  auto* object = SelectedObject(scene, editor_state.selection);
  if (object == nullptr) {
    return false;
  }

  switch (editor_state.gizmo_mode) {
    case GizmoMode::Translate:
      (&object->transform.translate.x)[index] += world_delta;
      return true;
    case GizmoMode::Scale:
      (&object->transform.scale.x)[index] = std::max(0.01F, (&object->transform.scale.x)[index] + world_delta);
      return true;
    case GizmoMode::Rotate:
      (&object->transform.rotate_deg.x)[index] += signed_pixels * 0.5F;
      return true;
  }
  return false;
}

auto DrawViewportOverlay(ray_tracer::scene::Scene& scene,
                         ray_tracer::scene_editor::EditorState& editor_state,
                         UiRenderState& render_state,
                         ImVec2 image_min,
                         ImVec2 image_size) -> OverlayResult {
  OverlayResult result;
  if (!scene.camera.has_value()) {
    return result;
  }

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 image_max{image_min.x + image_size.x, image_min.y + image_size.y};
  const ImVec2 mouse = ImGui::GetIO().MousePos;
  const bool mouse_in_image = ImGui::IsMouseHoveringRect(image_min, image_max);
  constexpr float kPickRadius = 18.0F;
  float closest_distance_sq = kPickRadius * kPickRadius;
  Selection closest_selection;
  bool marker_hovered = false;

  for (std::size_t i = 0; i < scene.objects.size(); ++i) {
    if (!scene.objects[i].visible) {
      continue;
    }
    const auto projected = ProjectWorldPoint(*scene.camera, ShapeCenter(scene.objects[i]), image_min, image_size);
    if (!projected.has_value()) {
      continue;
    }
    const bool selected = editor_state.selection.target == SelectionTarget::Object && editor_state.selection.index == i;
    const ImU32 color = selected ? IM_COL32(255, 225, 80, 255) : IM_COL32(255, 255, 255, 220);
    draw_list->AddCircleFilled(projected->screen, selected ? 6.0F : 4.0F, color);
    draw_list->AddCircle(projected->screen, 10.0F, color, 20, 1.5F);
    const std::string label = scene.objects[i].name + " (" + ObjectTypeLabel(scene.objects[i].type) + ")";
    draw_list->AddText(ImVec2(projected->screen.x + 10.0F, projected->screen.y - 10.0F), color, label.c_str());

    const float dist_sq = DistanceSquared(projected->screen, mouse);
    if (mouse_in_image && dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_selection = Selection{SelectionTarget::Object, i};
      marker_hovered = true;
    }
  }

  for (std::size_t i = 0; i < scene.lights.size(); ++i) {
    const auto projected = ProjectWorldPoint(*scene.camera, ToPoint(scene.lights[i].position), image_min, image_size);
    if (!projected.has_value()) {
      continue;
    }
    const bool selected = editor_state.selection.target == SelectionTarget::Light && editor_state.selection.index == i;
    const ImU32 color = selected ? IM_COL32(255, 225, 80, 255) : IM_COL32(255, 245, 160, 230);
    draw_list->AddRectFilled(ImVec2(projected->screen.x - 5.0F, projected->screen.y - 5.0F),
                             ImVec2(projected->screen.x + 5.0F, projected->screen.y + 5.0F), color);
    const std::string label = "Light " + std::to_string(i + 1);
    draw_list->AddText(ImVec2(projected->screen.x + 10.0F, projected->screen.y - 10.0F), color, label.c_str());

    const float dist_sq = DistanceSquared(projected->screen, mouse);
    if (mouse_in_image && dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_selection = Selection{SelectionTarget::Light, i};
      marker_hovered = true;
    }
  }

  std::optional<ray_tracer::Point> selected_center;
  if (editor_state.selection.target == SelectionTarget::Light && editor_state.selection.index < scene.lights.size()) {
    selected_center = ToPoint(scene.lights[editor_state.selection.index].position);
  } else if (editor_state.selection.target == SelectionTarget::Object &&
             editor_state.selection.index < scene.objects.size()) {
    selected_center = ShapeCenter(scene.objects[editor_state.selection.index]);
  }

  bool handle_hovered = false;
  bool handle_active = false;
  const ImVec2 restore_cursor = ImGui::GetCursorScreenPos();
  if (selected_center.has_value()) {
    const auto center_projected = ProjectWorldPoint(*scene.camera, *selected_center, image_min, image_size);
    if (center_projected.has_value()) {
      draw_list->AddCircleFilled(center_projected->screen, 5.0F, IM_COL32(255, 255, 255, 255));
      for (GizmoAxis axis : {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z}) {
        const auto endpoint = AxisEndpoint(scene, editor_state.selection, *selected_center, axis);
        const auto endpoint_projected = ProjectWorldPoint(*scene.camera, endpoint, image_min, image_size);
        if (!endpoint_projected.has_value()) {
          continue;
        }
        const ImU32 color = AxisColor(axis);
        draw_list->AddLine(center_projected->screen, endpoint_projected->screen, color, 3.0F);
        draw_list->AddCircleFilled(endpoint_projected->screen, 8.0F, color);
        draw_list->AddText(ImVec2(endpoint_projected->screen.x + 9.0F, endpoint_projected->screen.y - 8.0F),
                           color, AxisName(axis));

        ImGui::PushID(static_cast<int>(axis));
        ImGui::SetCursorScreenPos(ImVec2(endpoint_projected->screen.x - 10.0F, endpoint_projected->screen.y - 10.0F));
        ImGui::InvisibleButton("axis_handle", ImVec2(20.0F, 20.0F));
        handle_hovered |= ImGui::IsItemHovered();
        handle_active |= ImGui::IsItemActive();
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
          render_state.active_axis = axis;
          result.changed |= ApplyAxisDrag(scene, editor_state, axis, *selected_center, endpoint,
                                          center_projected->screen, endpoint_projected->screen,
                                          ImGui::GetIO().MouseDelta);
        }
        ImGui::PopID();
      }
    }
  }
  ImGui::SetCursorScreenPos(restore_cursor);

  if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    render_state.active_axis = GizmoAxis::None;
  }

  if (mouse_in_image && marker_hovered && !handle_hovered && !handle_active &&
      ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    editor_state.selection = closest_selection;
    result.block_camera = true;
  }

  result.block_camera |= marker_hovered || handle_hovered || handle_active || render_state.active_axis != GizmoAxis::None;
  return result;
}

auto SliderF(const char* label, float* value, float low, float high) -> bool {
  ImGui::SliderFloat(label, value, low, high);
  return ImGui::IsItemDeactivatedAfterEdit();
}

auto DragF3(const char* label, ray_tracer::scene::Vec3& value, float speed, float low, float high) -> bool {
  ImGui::DragFloat3(label, &value.x, speed, low, high);
  return ImGui::IsItemDeactivatedAfterEdit();
}

auto ColorE3(const char* label, ray_tracer::scene::ColorRgb& value) -> bool {
  ImGui::ColorEdit3(label, &value.r);
  return ImGui::IsItemDeactivatedAfterEdit();
}

auto PatternTypeIndex(ray_tracer::scene::PatternType type) -> int {
  return static_cast<int>(type);
}

auto PatternTypeFromIndex(int index) -> ray_tracer::scene::PatternType {
  using ray_tracer::scene::PatternType;
  if (index < 0 || index > static_cast<int>(PatternType::Perturbed)) {
    return PatternType::Solid;
  }
  return static_cast<PatternType>(index);
}

auto Solid(ray_tracer::scene::ColorRgb color) -> ray_tracer::scene::Pattern {
  ray_tracer::scene::Pattern pattern;
  pattern.type = ray_tracer::scene::PatternType::Solid;
  pattern.color = color;
  return pattern;
}

auto EnsurePatternChildren(ray_tracer::scene::Pattern& pattern) -> void {
  using ray_tracer::scene::PatternType;
  switch (pattern.type) {
    case PatternType::Solid:
      pattern.children.clear();
      break;
    case PatternType::Perturbed:
      if (pattern.children.size() != 1) {
        pattern.children = {Solid({1.0F, 1.0F, 1.0F})};
      }
      break;
    case PatternType::Stripe:
    case PatternType::Gradient:
    case PatternType::Ring:
    case PatternType::Checkers:
    case PatternType::RadialGradient:
    case PatternType::UvCheckers:
    case PatternType::Blended:
      if (pattern.children.size() != 2) {
        pattern.children = {Solid({1.0F, 1.0F, 1.0F}), Solid({0.0F, 0.0F, 0.0F})};
      }
      break;
  }
}

auto DrawPatternEditor(ray_tracer::scene::Pattern& pattern, int depth = 0) -> bool {
  bool changed = false;
  static const char* kPatternNames[] = {
      "Solid", "Stripe", "Gradient", "Ring", "Checkers", "Radial Gradient", "UV Checkers", "Blended", "Perturbed"};
  int pattern_index = PatternTypeIndex(pattern.type);
  ImGui::PushID(depth);
  if (ImGui::Combo("Type", &pattern_index, kPatternNames, IM_ARRAYSIZE(kPatternNames))) {
    pattern.type = PatternTypeFromIndex(pattern_index);
    EnsurePatternChildren(pattern);
    changed = true;
  }

  if (pattern.type == ray_tracer::scene::PatternType::Solid) {
    changed |= ColorE3("Color", pattern.color);
  } else {
    if (pattern.type == ray_tracer::scene::PatternType::Perturbed) {
      changed |= SliderF("Perturb Scale", &pattern.perturb_scale, 0.0F, 2.0F);
    }
    if (pattern.type == ray_tracer::scene::PatternType::UvCheckers) {
      ImGui::InputInt("Width", &pattern.width);
      changed |= ImGui::IsItemDeactivatedAfterEdit();
      ImGui::InputInt("Height", &pattern.height);
      changed |= ImGui::IsItemDeactivatedAfterEdit();
      pattern.width = std::max(1, pattern.width);
      pattern.height = std::max(1, pattern.height);
    }
    changed |= DragF3("Pattern Translate", pattern.transform.translate, 0.05F, -50.0F, 50.0F);
    changed |= DragF3("Pattern Scale", pattern.transform.scale, 0.01F, 0.01F, 20.0F);
    changed |= DragF3("Pattern Rotate", pattern.transform.rotate_deg, 1.0F, -360.0F, 360.0F);

    EnsurePatternChildren(pattern);
    for (std::size_t i = 0; i < pattern.children.size(); ++i) {
      const std::string label = pattern.type == ray_tracer::scene::PatternType::Perturbed
                                    ? "Source"
                                    : "Child " + std::to_string(i + 1);
      if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= DrawPatternEditor(pattern.children[i], depth + 1 + static_cast<int>(i));
        ImGui::TreePop();
      }
    }
  }
  ImGui::PopID();
  return changed;
}

auto DrawMaterialEditor(ray_tracer::scene::Material& material) -> bool {
  bool changed = false;
  changed |= ColorE3("Color", material.color);
  changed |= SliderF("Ambient", &material.ambient, 0.0F, 1.0F);
  changed |= SliderF("Diffuse", &material.diffuse, 0.0F, 1.0F);
  changed |= SliderF("Specular", &material.specular, 0.0F, 1.0F);
  changed |= SliderF("Shininess", &material.shininess, 1.0F, 500.0F);
  changed |= SliderF("Reflective", &material.reflective, 0.0F, 1.0F);
  changed |= SliderF("Transparency", &material.transparency, 0.0F, 1.0F);
  changed |= SliderF("Refractive Idx", &material.refractive_index, 0.1F, 3.0F);

  bool has_pattern = material.pattern.has_value();
  if (ImGui::Checkbox("Pattern", &has_pattern)) {
    if (has_pattern) {
      material.pattern = ray_tracer::scene::Pattern{};
    } else {
      material.pattern = std::nullopt;
    }
    changed = true;
  }
  if (material.pattern.has_value() && ImGui::TreeNodeEx("Pattern Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
    changed |= DrawPatternEditor(*material.pattern);
    ImGui::TreePop();
  }
  return changed;
}

auto DrawObjectEditor(ray_tracer::scene::Object& object) -> bool {
  bool changed = false;
  int type_index = object.type == ray_tracer::scene::ObjectType::Plane ? 1 : 0;
  static const char* kObjectTypeNames[] = {"Sphere", "Plane"};
  if (ImGui::Combo("Shape", &type_index, kObjectTypeNames, IM_ARRAYSIZE(kObjectTypeNames))) {
    object.type = type_index == 1 ? ray_tracer::scene::ObjectType::Plane : ray_tracer::scene::ObjectType::Sphere;
    changed = true;
  }
  if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    changed |= DragF3("Translate", object.transform.translate, 0.1F, -50.0F, 50.0F);
    changed |= DragF3("Scale", object.transform.scale, 0.01F, 0.01F, 20.0F);
    changed |= DragF3("Rotate (deg)", object.transform.rotate_deg, 1.0F, -360.0F, 360.0F);
    changed |= ImGui::Checkbox("Visible", &object.visible);
    changed |= ImGui::Checkbox("Cast Shadow", &object.cast_shadow);
    ImGui::TreePop();
  }
  if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
    changed |= DrawMaterialEditor(object.material);
    ImGui::TreePop();
  }
  return changed;
}

auto OrbitCamera(ray_tracer::scene::Camera& camera, float dx, float dy) -> void {
  float fx = camera.from.x - camera.to.x;
  float fy = camera.from.y - camera.to.y;
  float fz = camera.from.z - camera.to.z;
  const float radius = std::sqrt((fx * fx) + (fy * fy) + (fz * fz));
  if (radius < 0.001F) {
    return;
  }
  float theta = std::atan2(fx, fz);
  float phi = std::asin(std::clamp(fy / radius, -1.0F, 1.0F));
  theta -= dx * 0.01F;
  phi = std::clamp(phi + (dy * 0.01F), -1.5F, 1.5F);
  camera.from.x = camera.to.x + radius * std::sin(theta) * std::cos(phi);
  camera.from.y = camera.to.y + radius * std::sin(phi);
  camera.from.z = camera.to.z + radius * std::cos(theta) * std::cos(phi);
}

auto PanCamera(ray_tracer::scene::Camera& camera, float dx, float dy) -> void {
  float fx = camera.to.x - camera.from.x;
  float fy = camera.to.y - camera.from.y;
  float fz = camera.to.z - camera.from.z;
  const float len = std::sqrt((fx * fx) + (fy * fy) + (fz * fz));
  if (len < 0.001F) {
    return;
  }
  fx /= len;
  fy /= len;
  fz /= len;
  float rx = fz;
  float rz = -fx;
  const float right_len = std::sqrt((rx * rx) + (rz * rz));
  if (right_len < 0.001F) {
    return;
  }
  rx /= right_len;
  rz /= right_len;
  float ux = -rz * fy;
  float uy = (rz * fx) - (rx * fz);
  float uz = rx * fy;
  const float up_len = std::sqrt((ux * ux) + (uy * uy) + (uz * uz));
  if (up_len < 0.001F) {
    return;
  }
  ux /= up_len;
  uy /= up_len;
  uz /= up_len;

  const float speed = 0.005F * len;
  const float mx = ((-dx * rx) + (dy * ux)) * speed;
  const float my = (dy * uy) * speed;
  const float mz = ((-dx * rz) + (dy * uz)) * speed;
  camera.from.x += mx;
  camera.from.y += my;
  camera.from.z += mz;
  camera.to.x += mx;
  camera.to.y += my;
  camera.to.z += mz;
}

auto ZoomCamera(ray_tracer::scene::Camera& camera, float delta) -> void {
  const float fx = camera.from.x - camera.to.x;
  const float fy = camera.from.y - camera.to.y;
  const float fz = camera.from.z - camera.to.z;
  const float radius = std::sqrt((fx * fx) + (fy * fy) + (fz * fz));
  if (radius < 0.001F) {
    return;
  }
  const float new_radius = std::max(0.5F, radius * std::clamp(1.0F - (delta * 0.1F), 0.1F, 3.0F));
  const float scale = new_radius / radius;
  camera.from.x = camera.to.x + fx * scale;
  camera.from.y = camera.to.y + fy * scale;
  camera.from.z = camera.to.z + fz * scale;
}

auto CountVisibleObjects(const ray_tracer::scene::Scene& scene) -> std::size_t {
  return static_cast<std::size_t>(std::ranges::count_if(scene.objects, [](const ray_tracer::scene::Object& object) {
    return object.visible;
  }));
}

auto ObjectTypeName(ray_tracer::scene::ObjectType type) -> const char* {
  switch (type) {
    case ray_tracer::scene::ObjectType::Sphere:
      return "sphere";
    case ray_tracer::scene::ObjectType::Plane:
      return "plane";
  }
  return "unknown";
}

auto RuntimeShapeTypeName(const ray_tracer::Shape& shape) -> const char* {
  if (dynamic_cast<const ray_tracer::Sphere*>(&shape) != nullptr) {
    return "sphere";
  }
  if (dynamic_cast<const ray_tracer::Plane*>(&shape) != nullptr) {
    return "plane";
  }
  return "unknown";
}

auto LogAuthoringSceneSnapshot(const char* mode, const ray_tracer::scene::Scene& scene) -> void {
  std::fprintf(stderr,
               "[scene_editor] authoring snapshot mode=%s lights=%zu objects=%zu visible=%zu\n",
               mode,
               scene.lights.size(),
               scene.objects.size(),
               CountVisibleObjects(scene));
  for (std::size_t index = 0; index < scene.lights.size(); ++index) {
    const auto& light = scene.lights[index];
    std::fprintf(stderr,
                 "[scene_editor] authoring light mode=%s index=%zu position=[%.3f %.3f %.3f] color=[%.3f %.3f %.3f]\n",
                 mode,
                 index,
                 light.position.x,
                 light.position.y,
                 light.position.z,
                 light.color.r,
                 light.color.g,
                 light.color.b);
  }
  for (std::size_t index = 0; index < scene.objects.size(); ++index) {
    const auto& object = scene.objects[index];
    const auto& material = object.material;
    std::fprintf(stderr,
                 "[scene_editor] authoring object mode=%s index=%zu id=%zu name=\"%s\" type=%s visible=%d "
                 "cast_shadow=%d translate=[%.3f %.3f %.3f] rotate_deg=[%.3f %.3f %.3f] scale=[%.3f %.3f %.3f] "
                 "color=[%.3f %.3f %.3f] ambient=%.3f diffuse=%.3f specular=%.3f shininess=%.3f reflective=%.3f "
                 "transparency=%.3f refractive_index=%.3f pattern=%d\n",
                 mode,
                 index,
                 index + 1,
                 object.name.c_str(),
                 ObjectTypeName(object.type),
                 object.visible ? 1 : 0,
                 object.cast_shadow ? 1 : 0,
                 object.transform.translate.x,
                 object.transform.translate.y,
                 object.transform.translate.z,
                 object.transform.rotate_deg.x,
                 object.transform.rotate_deg.y,
                 object.transform.rotate_deg.z,
                 object.transform.scale.x,
                 object.transform.scale.y,
                 object.transform.scale.z,
                 material.color.r,
                 material.color.g,
                 material.color.b,
                 material.ambient,
                 material.diffuse,
                 material.specular,
                 material.shininess,
                 material.reflective,
                 material.transparency,
                 material.refractive_index,
                 material.pattern.has_value() ? 1 : 0);
  }
}

auto LogRuntimeSceneSnapshot(const char* mode,
                             int render_id,
                             const ray_tracer::scene_runtime::CompiledScene& compiled_scene) -> void {
  std::fprintf(stderr,
               "[scene_editor] runtime snapshot mode=%s id=%d lights=%zu objects=%zu camera=%dx%d\n",
               mode,
               render_id,
               compiled_scene.world.Lights().size(),
               compiled_scene.world.Objects().size(),
               compiled_scene.camera.GetHorizontalSize(),
               compiled_scene.camera.GetVerticalSize());
  for (std::size_t index = 0; index < compiled_scene.world.Lights().size(); ++index) {
    const auto& light = compiled_scene.world.Lights()[index];
    const auto& position = light.GetPosition();
    const auto& intensity = light.GetIntensity();
    std::fprintf(stderr,
                 "[scene_editor] runtime light mode=%s id=%d index=%zu position=[%.3f %.3f %.3f] color=[%.3f %.3f %.3f]\n",
                 mode,
                 render_id,
                 index,
                 position.GetX(),
                 position.GetY(),
                 position.GetZ(),
                 intensity.GetRed(),
                 intensity.GetGreen(),
                 intensity.GetBlue());
  }
  for (std::size_t index = 0; index < compiled_scene.world.Objects().size(); ++index) {
    const auto& shape = *compiled_scene.world.Objects()[index];
    const auto& material = shape.GetMaterial();
    const auto& color = material.GetColor();
    const auto world_origin = shape.GetTransform() * ray_tracer::Point(0, 0, 0);
    std::fprintf(stderr,
                 "[scene_editor] runtime object mode=%s id=%d index=%zu shape_id=%zu type=%s cast_shadow=%d "
                 "origin=[%.3f %.3f %.3f] color=[%.3f %.3f %.3f] ambient=%.3f diffuse=%.3f specular=%.3f "
                 "shininess=%.3f reflective=%.3f transparency=%.3f refractive_index=%.3f pattern=%d\n",
                 mode,
                 render_id,
                 index,
                 shape.GetId(),
                 RuntimeShapeTypeName(shape),
                 shape.GetCastShadow() ? 1 : 0,
                 world_origin.GetX(),
                 world_origin.GetY(),
                 world_origin.GetZ(),
                 color.GetRed(),
                 color.GetGreen(),
                 color.GetBlue(),
                 material.GetAmbient(),
                 material.GetDiffuse(),
                 material.GetSpecular(),
                 material.GetShininess(),
                 material.GetReflective(),
                 material.GetTransparency(),
                 material.GetRefractiveIndex(),
                 material.HasPattern() ? 1 : 0);
  }
}

auto MixFingerprint(std::uint64_t value, std::uint64_t input) -> std::uint64_t {
  value ^= input + 0x9e3779b97f4a7c15ULL + (value << 6U) + (value >> 2U);
  return value;
}

auto Quantize(float value) -> std::uint64_t {
  return static_cast<std::uint64_t>(static_cast<std::int64_t>(std::llround(value * 10000.0F)));
}

auto MixVec3(std::uint64_t value, const ray_tracer::scene::Vec3& vec) -> std::uint64_t {
  value = MixFingerprint(value, Quantize(vec.x));
  value = MixFingerprint(value, Quantize(vec.y));
  return MixFingerprint(value, Quantize(vec.z));
}

auto MixColor(std::uint64_t value, const ray_tracer::scene::ColorRgb& color) -> std::uint64_t {
  value = MixFingerprint(value, Quantize(color.r));
  value = MixFingerprint(value, Quantize(color.g));
  return MixFingerprint(value, Quantize(color.b));
}

auto MixTransform(std::uint64_t value, const ray_tracer::scene::Transform& transform) -> std::uint64_t {
  value = MixVec3(value, transform.translate);
  value = MixVec3(value, transform.rotate_deg);
  return MixVec3(value, transform.scale);
}

auto SceneFingerprint(const ray_tracer::scene::Scene& scene) -> std::uint64_t {
  std::uint64_t value = 1469598103934665603ULL;
  if (scene.camera.has_value()) {
    value = MixVec3(value, scene.camera->from);
    value = MixVec3(value, scene.camera->to);
    value = MixVec3(value, scene.camera->up);
    value = MixFingerprint(value, Quantize(scene.camera->fov_deg));
    value = MixFingerprint(value, static_cast<std::uint64_t>(scene.camera->resolution.width));
    value = MixFingerprint(value, static_cast<std::uint64_t>(scene.camera->resolution.height));
  }
  value = MixFingerprint(value, scene.lights.size());
  for (const auto& light : scene.lights) {
    value = MixVec3(value, light.position);
    value = MixColor(value, light.color);
  }
  value = MixFingerprint(value, scene.objects.size());
  for (const auto& object : scene.objects) {
    value = MixFingerprint(value, static_cast<std::uint64_t>(object.type));
    value = MixFingerprint(value, object.visible ? 1U : 0U);
    value = MixFingerprint(value, object.cast_shadow ? 1U : 0U);
    value = MixTransform(value, object.transform);
    value = MixColor(value, object.material.color);
    value = MixFingerprint(value, Quantize(object.material.ambient));
    value = MixFingerprint(value, Quantize(object.material.diffuse));
    value = MixFingerprint(value, Quantize(object.material.specular));
    value = MixFingerprint(value, Quantize(object.material.shininess));
    value = MixFingerprint(value, Quantize(object.material.reflective));
    value = MixFingerprint(value, object.material.pattern.has_value() ? 1U : 0U);
  }
  return value;
}

auto LogRayFingerprint(const char* label, int render_id, ray_tracer::Camera camera) -> void {
  const int width = camera.GetHorizontalSize();
  const int height = camera.GetVerticalSize();
  const auto top_left = camera.GetRayForPixel(0, 0);
  const auto center = camera.GetRayForPixel(width / 2, height / 2);
  const auto bottom_right = camera.GetRayForPixel(width - 1, height - 1);
  const auto& origin = center.GetOrigin();
  const auto& top_left_dir = top_left.GetDirection();
  const auto& center_dir = center.GetDirection();
  const auto& bottom_right_dir = bottom_right.GetDirection();
  std::fprintf(stderr,
               "[scene_editor] camera fingerprint %s id=%d res=%dx%d pixel=%.9f "
               "origin=[%.6f %.6f %.6f] tl_dir=[%.6f %.6f %.6f] center_dir=[%.6f %.6f %.6f] "
               "br_dir=[%.6f %.6f %.6f]\n",
               label,
               render_id,
               width,
               height,
               camera.GetPixelSize(),
               origin.GetX(),
               origin.GetY(),
               origin.GetZ(),
               top_left_dir.GetX(),
               top_left_dir.GetY(),
               top_left_dir.GetZ(),
               center_dir.GetX(),
               center_dir.GetY(),
               center_dir.GetZ(),
               bottom_right_dir.GetX(),
               bottom_right_dir.GetY(),
               bottom_right_dir.GetZ());
}

auto StopRender(UiRenderState& render_state) -> void {
  render_state.cancel.store(true);
  if (render_state.worker.joinable()) {
    std::fprintf(stderr, "[scene_editor] cancel render id=%d\n", render_state.active_render_id);
    render_state.worker.join();
  }
  render_state.rendering.store(false);
  render_state.cancel.store(false);
}

auto StartRender(UiRenderState& render_state,
                 const ray_tracer::scene_runtime::CompiledScene& compiled_scene,
                 bool preview) -> void {
  StopRender(render_state);
  const int render_id = render_state.next_render_id++;
  {
    std::lock_guard lock{render_state.image_mutex};
    render_state.pending_image = {};
    render_state.pending_render_id = 0;
    render_state.pending_is_preview = false;
    render_state.texture_dirty.store(false, std::memory_order_release);
  }
  render_state.cancel.store(false);
  render_state.progress.store(0);
  render_state.rendering.store(true);
  render_state.is_preview = preview;
  render_state.active_render_id = render_id;
  const int preview_divisor = preview ? ray_tracer::scene_editor::SceneRenderer::kPreviewDivisor : 1;
  std::fprintf(stderr, "[scene_editor] begin render id=%d mode=%s compiled=%dx%d objects=%zu lights=%zu preview_divisor=%d\n",
               render_id,
               preview ? "preview" : "full",
               compiled_scene.camera.GetHorizontalSize(),
               compiled_scene.camera.GetVerticalSize(),
               compiled_scene.world.Objects().size(),
               compiled_scene.world.Lights().size(),
               preview_divisor);
  LogRayFingerprint(preview ? "begin-preview" : "begin-full", render_id, compiled_scene.camera);
  LogRuntimeSceneSnapshot(preview ? "preview" : "full", render_id, compiled_scene);
  render_state.worker = std::thread([&render_state, compiled_scene, preview, render_id]() {
    const auto start = std::chrono::steady_clock::now();
    auto image = render_state.renderer.Render(compiled_scene, preview, &render_state.cancel, &render_state.progress);
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    if (!render_state.cancel.load() && !image.pixels.empty()) {
      std::lock_guard lock{render_state.image_mutex};
      render_state.pending_image = std::move(image);
      render_state.pending_render_id = render_id;
      render_state.pending_is_preview = preview;
      render_state.texture_dirty.store(true, std::memory_order_release);
      std::fprintf(stderr, "[scene_editor] finish render id=%d mode=%s image=%dx%d elapsed_ms=%lld\n",
                   render_id,
                   preview ? "preview" : "full",
                   render_state.pending_image.width,
                   render_state.pending_image.height,
                   static_cast<long long>(elapsed_ms));
    } else {
      std::fprintf(stderr, "[scene_editor] discard render id=%d mode=%s cancelled=%d elapsed_ms=%lld\n",
                   render_id,
                   preview ? "preview" : "full",
                   render_state.cancel.load() ? 1 : 0,
                   static_cast<long long>(elapsed_ms));
    }
    render_state.rendering.store(false);
  });
}

auto ScheduleRender(SceneEditorApp& app, UiRenderState& render_state, bool preview) -> void {
  if (!app.RecompileCurrentScene() || !app.CompiledScene().has_value()) {
    return;
  }
  app.ClearRenderRequest();
  if (preview) {
    render_state.last_interaction = std::chrono::steady_clock::now();
    render_state.high_res_pending = true;
  } else {
    render_state.high_res_pending = false;
  }
  const auto& scene = app.CurrentScene();
  LogAuthoringSceneSnapshot(preview ? "preview" : "full", scene);
  if (scene.camera.has_value()) {
    const auto& camera = *scene.camera;
    std::fprintf(stderr,
                 "[scene_editor] schedule mode=%s camera_from=[%.3f %.3f %.3f] camera_to=[%.3f %.3f %.3f] "
                 "fov=%.3f res=%dx%d scene_objects=%zu visible=%zu compiled_objects=%zu scene_hash=%016llx\n",
                 preview ? "preview" : "full",
                 camera.from.x,
                 camera.from.y,
                 camera.from.z,
                 camera.to.x,
                 camera.to.y,
                 camera.to.z,
                 camera.fov_deg,
                 camera.resolution.width,
                 camera.resolution.height,
                 scene.objects.size(),
                 CountVisibleObjects(scene),
                 app.CompiledScene()->world.Objects().size(),
                 static_cast<unsigned long long>(SceneFingerprint(scene)));
  }
  StartRender(render_state, *app.CompiledScene(), preview);
}

auto UploadPendingTexture(UiRenderState& render_state) -> void {
  if (render_state.rendering.load() || !render_state.texture_dirty.load(std::memory_order_acquire)) {
    return;
  }
  ray_tracer::scene_editor::RenderPixels image;
  int render_id = 0;
  bool is_preview = false;
  {
    std::lock_guard lock{render_state.image_mutex};
    image = std::move(render_state.pending_image);
    render_id = render_state.pending_render_id;
    is_preview = render_state.pending_is_preview;
    render_state.pending_render_id = 0;
    render_state.pending_is_preview = false;
    render_state.texture_dirty.store(false, std::memory_order_release);
  }
  if (image.pixels.empty()) {
    return;
  }
  const auto rgb = ray_tracer::scene_editor::SceneRenderer::ToRgb8(image);
  render_state.texture.UploadRgb(image.width, image.height, rgb);
  render_state.displayed_width = image.width;
  render_state.displayed_height = image.height;
  render_state.displayed_render_id = render_id;
  render_state.displayed_is_preview = is_preview;
  std::fprintf(stderr, "[scene_editor] upload render id=%d image=%dx%d aspect=%.6f\n",
               render_id,
               image.width,
               image.height,
               static_cast<double>(image.width) / static_cast<double>(image.height));
}

auto CopyPathToBuffer(const std::filesystem::path& path, std::array<char, 1024>& buffer) -> void {
  const auto text = path.string();
  buffer.fill('\0');
  const auto count = std::min(text.size(), buffer.size() - 1);
  std::copy_n(text.data(), count, buffer.data());
}

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
    if (app.SaveScene(path_buffer.data())) {
      CopyPathToBuffer(app.State().current_path, path_buffer);
    }
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
  if (!app.State().status_message.empty()) {
    ImGui::TextWrapped("%s", app.State().status_message.c_str());
  }
}

auto DrawTopLegend(const ray_tracer::scene::Scene& scene, ray_tracer::scene_editor::EditorState& state) -> void {
  ImGui::Text("Selected: %s", SelectedName(scene, state.selection).c_str());
  ImGui::SameLine();
  ImGui::Checkbox("Viewport Overlays", &state.show_viewport_overlays);
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.92F, 0.25F, 0.22F, 1.0F), "X");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.30F, 0.85F, 0.32F, 1.0F), "Y");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.30F, 0.55F, 0.96F, 1.0F), "Z");

  if (ImGui::RadioButton("Move", state.gizmo_mode == GizmoMode::Translate)) {
    state.gizmo_mode = GizmoMode::Translate;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Scale", state.gizmo_mode == GizmoMode::Scale)) {
    state.gizmo_mode = GizmoMode::Scale;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Rotate", state.gizmo_mode == GizmoMode::Rotate)) {
    state.gizmo_mode = GizmoMode::Rotate;
  }
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
    scene.camera = ray_tracer::scene::Camera{};
    changed = true;
  }
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::SmallButton(editor_state.selection.target == SelectionTarget::Camera ? "Selected" : "Select")) {
      editor_state.selection = Selection{SelectionTarget::Camera, 0};
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
      if (ImGui::SmallButton(editor_state.selection.target == SelectionTarget::Light && editor_state.selection.index == i
                                 ? "Selected"
                                 : "Select")) {
        editor_state.selection = Selection{SelectionTarget::Light, i};
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
      if (editor_state.selection.target == SelectionTarget::Light) {
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
      scene.lights.push_back(ray_tracer::scene::Light{});
      editor_state.selection = Selection{SelectionTarget::Light, scene.lights.size() - 1};
      changed = true;
    }
  }

  ImGui::Separator();
  ImGui::Text("Objects (%zu)", scene.objects.size());
  int delete_index = -1;
  for (std::size_t i = 0; i < scene.objects.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    const bool selected = editor_state.selection.target == SelectionTarget::Object && editor_state.selection.index == i;
    if (ImGui::SmallButton(selected ? "Selected" : "Select")) {
      editor_state.selection = Selection{SelectionTarget::Object, i};
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
    if (editor_state.selection.target == SelectionTarget::Object) {
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

auto DrawAddPanel(SceneEditorApp& app, UiRenderState& render_state) -> bool {
  auto& scene = app.MutableScene();
  auto& state = app.MutableState();
  bool changed = false;
  if (ImGui::Button("+ Sphere", ImVec2(-1.0F, 40.0F))) {
    ray_tracer::scene::Object object;
    object.name = "Sphere " + std::to_string(scene.objects.size() + 1);
    object.type = ray_tracer::scene::ObjectType::Sphere;
    object.transform.translate.y = 1.0F;
    scene.objects.push_back(object);
    state.selection = Selection{SelectionTarget::Object, scene.objects.size() - 1};
    changed = true;
  }
  if (ImGui::Button("+ Plane", ImVec2(-1.0F, 40.0F))) {
    ray_tracer::scene::Object object;
    object.name = "Plane " + std::to_string(scene.objects.size() + 1);
    object.type = ray_tracer::scene::ObjectType::Plane;
    scene.objects.push_back(object);
    state.selection = Selection{SelectionTarget::Object, scene.objects.size() - 1};
    changed = true;
  }
  ImGui::Separator();
  ImGui::SliderFloat("Full-res delay", &render_state.high_res_delay, 0.5F, 10.0F, "%.1fs");
  return changed;
}

}  // namespace

auto main(int argc, char** argv) -> int {
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    return 1;
  }

  const auto* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(1600, 900, "Scene Editor", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  SceneEditorApp app;
  UiRenderState render_state;
  std::array<char, 1024> path_buffer{};
  const bool loaded = argc > 1 ? app.LoadScene(argv[1]) : app.LoadDefaultScene();
  CopyPathToBuffer(app.State().current_path, path_buffer);
  if (loaded && app.CompiledScene().has_value()) {
    app.ClearRenderRequest();
    StartRender(render_state, *app.CompiledScene(), false);
  }

  constexpr auto kClearColor = ImVec4(0.15F, 0.15F, 0.18F, 1.00F);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    UploadPendingTexture(render_state);
    bool changed = false;
    bool camera_moved = false;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(430, 900), ImGuiCond_Once);
    ImGui::Begin("Scene");
    changed |= DrawScenePanel(app, render_state, path_buffer);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(1410, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(190, 900), ImGuiCond_Once);
    ImGui::Begin("Add");
    changed |= DrawAddPanel(app, render_state);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(435, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(970, 900), ImGuiCond_Once);
    ImGui::Begin("Viewport");
    DrawTopLegend(app.CurrentScene(), app.MutableState());
    ImGui::Separator();

    if (render_state.texture.Valid() && render_state.displayed_width > 0 && render_state.displayed_height > 0) {
      const ImVec2 avail = ImGui::GetContentRegionAvail();
      const float image_width = static_cast<float>(render_state.displayed_width);
      const float image_height = static_cast<float>(render_state.displayed_height);
      float scale = std::min(avail.x / image_width, avail.y / image_height);
      if (scale <= 0.0F) {
        scale = 1.0F;
      }
      const ImVec2 image_size{image_width * scale, image_height * scale};
      float cursor_x = ImGui::GetCursorPosX();
      float cursor_y = ImGui::GetCursorPosY();
      if (avail.x > image_size.x) {
        cursor_x += (avail.x - image_size.x) * 0.5F;
      }
      if (avail.y > image_size.y) {
        cursor_y += (avail.y - image_size.y) * 0.5F;
      }
      ImGui::SetCursorPos(ImVec2(cursor_x, cursor_y));
      // NOLINTNEXTLINE(performance-no-int-to-ptr): ImGui stores an opaque OpenGL handle.
      ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(render_state.texture.Id())), image_size);
      const ImVec2 image_min = ImGui::GetItemRectMin();
      const ImVec2 image_max = ImGui::GetItemRectMax();
      auto& scene = app.MutableScene();
      OverlayResult overlay;
      if (app.State().show_viewport_overlays) {
        overlay = DrawViewportOverlay(scene, app.MutableState(), render_state, image_min, image_size);
      }
      changed |= overlay.changed;

      if (scene.camera.has_value() && ImGui::IsMouseHoveringRect(image_min, image_max) && !overlay.block_camera) {
        const ImVec2 drag = ImGui::GetIO().MouseDelta;
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
          OrbitCamera(*scene.camera, drag.x, drag.y);
          camera_moved = true;
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
          PanCamera(*scene.camera, drag.x, drag.y);
          camera_moved = true;
        }
        const float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0F) {
          ZoomCamera(*scene.camera, scroll);
          camera_moved = true;
        }
      }
    } else {
      ImGui::Text("No render yet");
    }

    if (render_state.is_preview && !render_state.rendering.load()) {
      const auto elapsed = std::chrono::duration<double>(
          std::chrono::steady_clock::now() - render_state.last_interaction).count();
      ImGui::TextDisabled("Full render in %.1fs", std::max(0.0, static_cast<double>(render_state.high_res_delay) - elapsed));
    }
    ImGui::End();

    if (changed || camera_moved) {
      render_state.last_interaction = std::chrono::steady_clock::now();
      render_state.high_res_pending = true;
      ScheduleRender(app, render_state, true);
    }

    if (!render_state.rendering.load() && app.State().render_state.render_requested) {
      ScheduleRender(app, render_state, true);
    }

    if (render_state.high_res_pending && !render_state.rendering.load()) {
      const auto elapsed = std::chrono::duration<double>(
          std::chrono::steady_clock::now() - render_state.last_interaction).count();
      if (elapsed >= static_cast<double>(render_state.high_res_delay)) {
        ScheduleRender(app, render_state, false);
      }
    }

    ImGui::Render();
    int display_width = 0;
    int display_height = 0;
    glfwGetFramebufferSize(window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);
    glClearColor(kClearColor.x * kClearColor.w, kClearColor.y * kClearColor.w,
                 kClearColor.z * kClearColor.w, kClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  StopRender(render_state);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
