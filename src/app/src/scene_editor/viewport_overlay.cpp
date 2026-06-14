#include "app/scene_editor/viewport_overlay.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <optional>
#include <string>

#include "imgui.h"  // NOLINT

#include "app/matrix.h"
#include "app/point.h"
#include "app/scene/scene.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/transformations.h"
#include "app/vec3.h"

namespace ray_tracer::scene_editor {
namespace {

struct ProjectedPoint {
  ImVec2 screen;
  float depth{0.0F};
};

auto ToPoint(const scene::Vec3& value) -> Point {
  return {value.x, value.y, value.z};
}

auto BuildTransformMatrix(const scene::Transform& transform) -> Matrix<4, 4> {
  auto matrix = Matrix<4, 4>::Translation(transform.translate.x, transform.translate.y, transform.translate.z);
  constexpr auto kDegToRad = std::numbers::pi / 180.0;
  if (transform.rotate_deg.x != 0.0F) {
    matrix = matrix * Matrix<4, 4>::RotationX(transform.rotate_deg.x * kDegToRad);
  }
  if (transform.rotate_deg.y != 0.0F) {
    matrix = matrix * Matrix<4, 4>::RotationY(transform.rotate_deg.y * kDegToRad);
  }
  if (transform.rotate_deg.z != 0.0F) {
    matrix = matrix * Matrix<4, 4>::RotationZ(transform.rotate_deg.z * kDegToRad);
  }
  return matrix * Matrix<4, 4>::Scaling(transform.scale.x, transform.scale.y, transform.scale.z);
}

auto SelectedObject(scene::Scene& scene, const Selection& selection) -> scene::Object* {
  if (selection.target != SelectionTarget::kObject || selection.index >= scene.objects.size()) {
    return nullptr;
  }
  return &scene.objects[selection.index];
}

auto ShapeCenter(const scene::Object& object) -> Point {
  return {BuildTransformMatrix(object.transform) * Point(0, 0, 0)};
}

auto DistanceSquared(ImVec2 lhs, ImVec2 rhs) -> float {
  const float delta_x = lhs.x - rhs.x;
  const float delta_y = lhs.y - rhs.y;
  return (delta_x * delta_x) + (delta_y * delta_y);
}

auto AxisName(GizmoAxis axis) -> const char* {
  switch (axis) {
    case GizmoAxis::kX:
      return "X";
    case GizmoAxis::kY:
      return "Y";
    case GizmoAxis::kZ:
      return "Z";
    case GizmoAxis::kNone:
      return "-";
  }
  return "-";
}

auto AxisIndex(GizmoAxis axis) -> int {
  switch (axis) {
    case GizmoAxis::kX:
      return 0;
    case GizmoAxis::kY:
      return 1;
    case GizmoAxis::kZ:
      return 2;
    case GizmoAxis::kNone:
      return -1;
  }
  return -1;
}

auto AxisColor(GizmoAxis axis) -> ImU32 {
  switch (axis) {
    case GizmoAxis::kX:
      return IM_COL32(235, 65, 55, 255);
    case GizmoAxis::kY:
      return IM_COL32(80, 210, 85, 255);
    case GizmoAxis::kZ:
      return IM_COL32(80, 140, 245, 255);
    case GizmoAxis::kNone:
      return IM_COL32(255, 255, 255, 255);
  }
  return IM_COL32(255, 255, 255, 255);
}

auto ProjectWorldPoint(const scene::Camera& camera,
                       const Point& world_point,
                       ImVec2 image_min,
                       ImVec2 image_size) -> std::optional<ProjectedPoint> {
  const auto view = ViewTransform(ToPoint(camera.from), ToPoint(camera.to), Vec3(camera.up.x, camera.up.y, camera.up.z));
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

  const float screen_x = image_min.x + static_cast<float>((half_width - projected_x) / (2.0 * half_width)) * image_size.x;
  const float screen_y = image_min.y + static_cast<float>((half_height - projected_y) / (2.0 * half_height)) * image_size.y;
  return ProjectedPoint{ImVec2(screen_x, screen_y), static_cast<float>(depth)};
}

auto AxisEndpoint(const scene::Scene& scene,
                  const Selection& selection,
                  const Point& center,
                  GizmoAxis axis) -> Point {
  constexpr double kAxisLength = 1.25;
  if (selection.target == SelectionTarget::kObject && selection.index < scene.objects.size()) {
    const auto transform = BuildTransformMatrix(scene.objects[selection.index].transform);
    switch (axis) {
      case GizmoAxis::kX:
        return {transform * Point(kAxisLength, 0, 0)};
      case GizmoAxis::kY:
        return {transform * Point(0, kAxisLength, 0)};
      case GizmoAxis::kZ:
        return {transform * Point(0, 0, kAxisLength)};
      case GizmoAxis::kNone:
        return center;
    }
  }

  switch (axis) {
    case GizmoAxis::kX:
      return {center.GetX() + kAxisLength, center.GetY(), center.GetZ()};
    case GizmoAxis::kY:
      return {center.GetX(), center.GetY() + kAxisLength, center.GetZ()};
    case GizmoAxis::kZ:
      return {center.GetX(), center.GetY(), center.GetZ() + kAxisLength};
    case GizmoAxis::kNone:
      return center;
  }
  return center;
}

auto ApplyAxisDrag(scene::Scene& scene,
                   EditorState& editor_state,
                   GizmoAxis axis,
                   const Point& center,
                   const Point& endpoint,
                   ImVec2 center_screen,
                   ImVec2 endpoint_screen,
                   ImVec2 mouse_delta) -> bool {
  const int index = AxisIndex(axis);
  if (index < 0) {
    return false;
  }

  const float screen_x = endpoint_screen.x - center_screen.x;
  const float screen_y = endpoint_screen.y - center_screen.y;
  const float screen_length = std::sqrt((screen_x * screen_x) + (screen_y * screen_y));
  if (screen_length < 3.0F) {
    return false;
  }

  const double world_x = endpoint.GetX() - center.GetX();
  const double world_y = endpoint.GetY() - center.GetY();
  const double world_z = endpoint.GetZ() - center.GetZ();
  const auto world_length = static_cast<float>(std::sqrt((world_x * world_x) + (world_y * world_y) + (world_z * world_z)));
  const float signed_pixels = ((mouse_delta.x * screen_x) + (mouse_delta.y * screen_y)) / screen_length;
  const float world_delta = signed_pixels * std::max(world_length, 0.1F) / screen_length;

  if (editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index < scene.lights.size()) {
    auto& position = scene.lights[editor_state.selection.index].position;
    (&position.x)[index] += world_delta;
    return true;
  }

  auto* object = SelectedObject(scene, editor_state.selection);
  if (object == nullptr) {
    return false;
  }

  switch (editor_state.gizmo_mode) {
    case GizmoMode::kTranslate:
      (&object->transform.translate.x)[index] += world_delta;
      return true;
    case GizmoMode::kScale:
      (&object->transform.scale.x)[index] = std::max(0.01F, (&object->transform.scale.x)[index] + world_delta);
      return true;
    case GizmoMode::kRotate:
      (&object->transform.rotate_deg.x)[index] += signed_pixels * 0.5F;
      return true;
  }
  return false;
}

}  // namespace

auto ObjectTypeLabel(scene::ObjectType type) -> const char* {
  return type == scene::ObjectType::kPlane ? "Plane" : "Sphere";
}

auto SelectedName(const scene::Scene& scene, const Selection& selection) -> std::string {
  if (selection.target == SelectionTarget::kCamera) {
    return "Camera";
  }
  if (selection.target == SelectionTarget::kLight && selection.index < scene.lights.size()) {
    return "Light " + std::to_string(selection.index + 1);
  }
  if (selection.target == SelectionTarget::kObject && selection.index < scene.objects.size()) {
    return scene.objects[selection.index].name;
  }
  return "None";
}

auto DrawViewportOverlay(scene::Scene& scene,
                         EditorState& editor_state,
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
    const bool selected = editor_state.selection.target == SelectionTarget::kObject && editor_state.selection.index == i;
    const ImU32 color = selected ? IM_COL32(255, 225, 80, 255) : IM_COL32(255, 255, 255, 220);
    draw_list->AddCircleFilled(projected->screen, selected ? 6.0F : 4.0F, color);
    draw_list->AddCircle(projected->screen, 10.0F, color, 20, 1.5F);
    const std::string label = scene.objects[i].name + " (" + ObjectTypeLabel(scene.objects[i].type) + ")";
    draw_list->AddText(ImVec2(projected->screen.x + 10.0F, projected->screen.y - 10.0F), color, label.c_str());

    const float dist_sq = DistanceSquared(projected->screen, mouse);
    if (mouse_in_image && dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_selection = Selection{SelectionTarget::kObject, i};
      marker_hovered = true;
    }
  }

  for (std::size_t i = 0; i < scene.lights.size(); ++i) {
    const auto projected = ProjectWorldPoint(*scene.camera, ToPoint(scene.lights[i].position), image_min, image_size);
    if (!projected.has_value()) {
      continue;
    }
    const bool selected = editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index == i;
    const ImU32 color = selected ? IM_COL32(255, 225, 80, 255) : IM_COL32(255, 245, 160, 230);
    draw_list->AddRectFilled(ImVec2(projected->screen.x - 5.0F, projected->screen.y - 5.0F),
                             ImVec2(projected->screen.x + 5.0F, projected->screen.y + 5.0F), color);
    const std::string label = "Light " + std::to_string(i + 1);
    draw_list->AddText(ImVec2(projected->screen.x + 10.0F, projected->screen.y - 10.0F), color, label.c_str());

    const float dist_sq = DistanceSquared(projected->screen, mouse);
    if (mouse_in_image && dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_selection = Selection{SelectionTarget::kLight, i};
      marker_hovered = true;
    }
  }

  std::optional<Point> selected_center;
  if (editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index < scene.lights.size()) {
    selected_center = ToPoint(scene.lights[editor_state.selection.index].position);
  } else if (editor_state.selection.target == SelectionTarget::kObject && editor_state.selection.index < scene.objects.size()) {
    selected_center = ShapeCenter(scene.objects[editor_state.selection.index]);
  }

  bool handle_hovered = false;
  bool handle_active = false;
  const ImVec2 restore_cursor = ImGui::GetCursorScreenPos();
  if (selected_center.has_value()) {
    const auto center_projected = ProjectWorldPoint(*scene.camera, *selected_center, image_min, image_size);
    if (center_projected.has_value()) {
      draw_list->AddCircleFilled(center_projected->screen, 5.0F, IM_COL32(255, 255, 255, 255));
      for (const GizmoAxis axis : {GizmoAxis::kX, GizmoAxis::kY, GizmoAxis::kZ}) {
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
  ImGui::Dummy(ImVec2(0.0F, 0.0F));

  if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    render_state.active_axis = GizmoAxis::kNone;
  }

  if (mouse_in_image && marker_hovered && !handle_hovered && !handle_active &&
      ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    editor_state.selection = closest_selection;
    result.block_camera = true;
  }

  result.block_camera |= marker_hovered || handle_hovered || handle_active || render_state.active_axis != GizmoAxis::kNone;
  return result;
}

}  // namespace ray_tracer::scene_editor
