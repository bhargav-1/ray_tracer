#include "app/scene_editor/viewport_overlay.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

#include "imgui.h"  // NOLINT

#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/transformations.h"
#include "app/primitives/vec3.h"
#include "app/scene/scene.h"
#include "app/scene/shape_catalog.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"

namespace ray_tracer::scene_editor {
namespace {

struct ProjectedPoint {
  ImVec2 screen;
  float depth{0.0F};
};

using ObjectPath = std::vector<std::size_t>;

struct ObjectEntry {
  scene::Object* object;
  ObjectPath path;
  Matrix<4, 4> world_transform;
};

auto ToPoint(const scene::Vec3& value) -> Point { return {value.x, value.y, value.z}; }

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
      if (!object->IsContainer()) {
        return nullptr;
      }
      current_objects = &object->Children();
    }
  }
  return object;
}

auto FindObject(const std::vector<scene::Object>& objects, const ObjectPath& path) -> const scene::Object* {
  const auto* current_objects = &objects;
  const scene::Object* object = nullptr;
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

auto SelectedObject(scene::Scene& scene, const Selection& selection) -> scene::Object* {
  if (selection.target != SelectionTarget::kObject) {
    return nullptr;
  }
  return FindObject(scene.objects, selection.object_path);
}

auto CollectObjects(std::vector<scene::Object>& objects,  // NOLINT(misc-no-recursion)
                    const Matrix<4, 4>& parent_transform,
                    const ObjectPath& parent_path,
                    std::vector<ObjectEntry>& entries) -> void {
  for (std::size_t index = 0; index < objects.size(); ++index) {
    auto& object = objects[index];
    if (!object.visible) {
      continue;
    }
    auto path = parent_path;
    path.push_back(index);
    const auto world_transform = parent_transform * BuildTransformMatrix(object.transform);
    entries.push_back(ObjectEntry{&object, path, world_transform});
    if (object.IsContainer()) {
      CollectObjects(object.Children(), world_transform, path, entries);
    }
  }
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
  const auto view =
      ViewTransform(ToPoint(camera.from), ToPoint(camera.to), Vec3(camera.up.x, camera.up.y, camera.up.z));
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

  const float screen_x =
      image_min.x + static_cast<float>((half_width - projected_x) / (2.0 * half_width)) * image_size.x;
  const float screen_y =
      image_min.y + static_cast<float>((half_height - projected_y) / (2.0 * half_height)) * image_size.y;
  return ProjectedPoint{ImVec2(screen_x, screen_y), static_cast<float>(depth)};
}

auto AxisEndpoint(const Matrix<4, 4>& world_transform, const Point& center, GizmoAxis axis) -> Point {
  constexpr double kAxisLength = 1.25;
  switch (axis) {
    case GizmoAxis::kX:
      return {world_transform * Point(kAxisLength, 0, 0)};
    case GizmoAxis::kY:
      return {world_transform * Point(0, kAxisLength, 0)};
    case GizmoAxis::kZ:
      return {world_transform * Point(0, 0, kAxisLength)};
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
  const auto world_length =
      static_cast<float>(std::sqrt((world_x * world_x) + (world_y * world_y) + (world_z * world_z)));
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

auto ObjectTypeLabel(scene::ObjectType type) -> std::string { return shape_catalog::DisplayName(type); }

auto SelectedName(const scene::Scene& scene, const Selection& selection) -> std::string {
  if (selection.target == SelectionTarget::kCamera) {
    return "Camera";
  }
  if (selection.target == SelectionTarget::kLight && selection.index < scene.lights.size()) {
    return "Light " + std::to_string(selection.index + 1);
  }
  if (selection.target == SelectionTarget::kObject) {
    if (const auto* object = FindObject(scene.objects, selection.object_path)) {
      return object->name;
    }
  }
  return "None";
}

auto DrawViewportOverlay(scene::Scene& scene,
                         EditorState& editor_state,
                         UiRenderState& render_state,
                         ImVec2 image_min,
                         ImVec2 image_size,
                         bool image_hovered) -> OverlayResult {
  OverlayResult result;
  if (!scene.camera.has_value()) {
    return result;
  }

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 mouse = ImGui::GetIO().MousePos;
  const bool mouse_in_image = image_hovered;
  constexpr float kPickRadius = 18.0F;
  float closest_distance_sq = kPickRadius * kPickRadius;
  Selection closest_selection;
  bool marker_hovered = false;

  std::vector<ObjectEntry> object_entries;
  CollectObjects(scene.objects, Matrix<4, 4>::IdentityMatrix(), {}, object_entries);
  for (const auto& entry : object_entries) {
    if (entry.object->IsContainer()) {
      continue;
    }
    const bool selected =
        editor_state.selection.target == SelectionTarget::kObject && editor_state.selection.object_path == entry.path;
    if (!selected) {
      continue;
    }
    const auto center = Point{entry.world_transform * Point(0, 0, 0)};
    const auto projected = ProjectWorldPoint(*scene.camera, center, image_min, image_size);
    if (!projected.has_value()) {
      continue;
    }
    const ImU32 color = IM_COL32(255, 225, 80, 255);
    draw_list->AddCircleFilled(projected->screen, 6.0F, color);
    draw_list->AddCircle(projected->screen, 10.0F, color, 20, 1.5F);
    const std::string label = entry.object->name + " (" + ObjectTypeLabel(entry.object->Shape().type) + ")";
    draw_list->AddText(ImVec2(projected->screen.x + 10.0F, projected->screen.y - 10.0F), color, label.c_str());

    const float dist_sq = DistanceSquared(projected->screen, mouse);
    if (mouse_in_image && dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_selection = Selection::ForObject(entry.path);
      marker_hovered = true;
    }
  }

  for (std::size_t i = 0; i < scene.lights.size(); ++i) {
    if (!scene.lights[i].visible) {
      continue;
    }
    const bool selected = editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index == i;
    if (!selected) {
      continue;
    }
    const auto projected = ProjectWorldPoint(*scene.camera, ToPoint(scene.lights[i].position), image_min, image_size);
    if (!projected.has_value()) {
      continue;
    }
    const ImU32 color = IM_COL32(255, 225, 80, 255);
    draw_list->AddRectFilled(ImVec2(projected->screen.x - 5.0F, projected->screen.y - 5.0F),
                             ImVec2(projected->screen.x + 5.0F, projected->screen.y + 5.0F),
                             color);
    const std::string label = "Light " + std::to_string(i + 1);
    draw_list->AddText(ImVec2(projected->screen.x + 10.0F, projected->screen.y - 10.0F), color, label.c_str());

    const float dist_sq = DistanceSquared(projected->screen, mouse);
    if (mouse_in_image && dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_selection = Selection::ForLight(i);
      marker_hovered = true;
    }
  }

  std::optional<Point> selected_center;
  auto selected_transform = Matrix<4, 4>::IdentityMatrix();
  if (editor_state.selection.target == SelectionTarget::kLight && editor_state.selection.index < scene.lights.size() &&
      scene.lights[editor_state.selection.index].visible) {
    selected_center = ToPoint(scene.lights[editor_state.selection.index].position);
    selected_transform =
        Matrix<4, 4>::Translation(selected_center->GetX(), selected_center->GetY(), selected_center->GetZ());
  } else if (editor_state.selection.target == SelectionTarget::kObject) {
    const auto selected_entry =
        std::find_if(object_entries.begin(), object_entries.end(), [&](const ObjectEntry& entry) {
          return entry.path == editor_state.selection.object_path;
        });
    if (selected_entry != object_entries.end()) {
      selected_transform = selected_entry->world_transform;
      selected_center = Point{selected_transform * Point(0, 0, 0)};
    }
  }

  bool handle_hovered = false;
  bool handle_active = false;
  const ImVec2 restore_cursor = ImGui::GetCursorScreenPos();
  if (selected_center.has_value()) {
    const auto center_projected = ProjectWorldPoint(*scene.camera, *selected_center, image_min, image_size);
    if (center_projected.has_value()) {
      draw_list->AddCircleFilled(center_projected->screen, 5.0F, IM_COL32(255, 255, 255, 255));
      for (const GizmoAxis axis : {GizmoAxis::kX, GizmoAxis::kY, GizmoAxis::kZ}) {
        const auto endpoint = AxisEndpoint(selected_transform, *selected_center, axis);
        const auto endpoint_projected = ProjectWorldPoint(*scene.camera, endpoint, image_min, image_size);
        if (!endpoint_projected.has_value()) {
          continue;
        }
        const ImU32 color = AxisColor(axis);
        draw_list->AddLine(center_projected->screen, endpoint_projected->screen, color, 3.0F);
        draw_list->AddCircleFilled(endpoint_projected->screen, 8.0F, color);
        draw_list->AddText(
            ImVec2(endpoint_projected->screen.x + 9.0F, endpoint_projected->screen.y - 8.0F), color, AxisName(axis));

        ImGui::PushID(static_cast<int>(axis));
        ImGui::SetCursorScreenPos(ImVec2(endpoint_projected->screen.x - 10.0F, endpoint_projected->screen.y - 10.0F));
        ImGui::InvisibleButton("axis_handle", ImVec2(20.0F, 20.0F));
        handle_hovered |= ImGui::IsItemHovered();
        handle_active |= ImGui::IsItemActive();
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
          render_state.active_axis = axis;
          result.changed |= ApplyAxisDrag(scene,
                                          editor_state,
                                          axis,
                                          *selected_center,
                                          endpoint,
                                          center_projected->screen,
                                          endpoint_projected->screen,
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

  result.block_camera |=
      marker_hovered || handle_hovered || handle_active || render_state.active_axis != GizmoAxis::kNone;
  return result;
}

}  // namespace ray_tracer::scene_editor
