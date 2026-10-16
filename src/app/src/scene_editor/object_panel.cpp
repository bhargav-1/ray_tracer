#include "app/scene_editor/object_panel.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include "imgui.h"  // NOLINT

#include "app/scene/scene.h"
#include "app/scene_editor/widget_helpers.h"
#include "app/scene_editor/imgui_file_dialog.h"
#include "app/scene/shape_catalog.h"

namespace ray_tracer::scene_editor {
namespace {

auto PatternTypeIndex(scene::PatternType type) -> int {
  return static_cast<int>(type);
}

auto PatternTypeFromIndex(int index) -> scene::PatternType {
  using scene::PatternType;
  if (index < 0 || index > static_cast<int>(PatternType::kPerturbed)) {
    return PatternType::kSolid;
  }
  return static_cast<PatternType>(index);
}

auto Solid(scene::ColorRgb color) -> scene::Pattern {
  scene::Pattern pattern;
  pattern.type = scene::PatternType::kSolid;
  pattern.color = color;
  return pattern;
}

auto EnsurePatternChildren(scene::Pattern& pattern) -> void {
  using scene::PatternType;
  switch (pattern.type) {
    case PatternType::kSolid:
      pattern.children.clear();
      break;
    case PatternType::kPerturbed:
      if (pattern.children.size() != 1) {
        pattern.children = {Solid({1.0F, 1.0F, 1.0F})};
      }
      break;
    case PatternType::kStripe:
    case PatternType::kGradient:
    case PatternType::kRing:
    case PatternType::kCheckers:
    case PatternType::kRadialGradient:
    case PatternType::kUvCheckers:
    case PatternType::kBlended:
      if (pattern.children.size() != 2) {
        pattern.children = {Solid({1.0F, 1.0F, 1.0F}), Solid({0.0F, 0.0F, 0.0F})};
      }
      break;
  }
}

auto DrawAxialBoundsEditor(scene::AxialBounds& bounds) -> bool {
  constexpr auto kMinimumGap = 0.01F;
  bool changed = false;
  if (ImGui::DragFloat("Minimum", &bounds.minimum, 0.05F)) {
    bounds.minimum = std::min(bounds.minimum, bounds.maximum - kMinimumGap);
    changed = true;
  }
  if (ImGui::DragFloat("Maximum", &bounds.maximum, 0.05F)) {
    bounds.maximum = std::max(bounds.maximum, bounds.minimum + kMinimumGap);
    changed = true;
  }
  changed |= ImGui::Checkbox("Closed", &bounds.closed);
  return changed;
}

// NOLINTNEXTLINE(misc-no-recursion)
auto PreserveCsgOverrides(scene::Object& operand, const scene::CsgSpec& csg) -> void {
  if (operand.IsGroup()) {
    for (auto& child : operand.Children()) {
      PreserveCsgOverrides(child, csg);
    }
  } else if (operand.IsCsg()) {
    if (csg.material.has_value()) {
      operand.Csg().material = csg.material;
    }
    operand.Csg().cast_shadow &= csg.cast_shadow;
  } else {
    if (csg.material.has_value()) {
      operand.Shape().material = *csg.material;
    }
    operand.Shape().cast_shadow &= csg.cast_shadow;
  }
}

}  // namespace

auto UnwrapCsg(scene::Object& object, const std::size_t operand_index) -> bool {
  if (!object.IsCsg() || object.Csg().children.size() != 2 || operand_index >= 2) {
    return false;
  }
  auto kept = std::move(object.Csg().children[operand_index]);
  PreserveCsgOverrides(kept, object.Csg());
  const auto outer_transform = object.transform;
  const bool visible = object.visible;
  if (outer_transform == scene::Transform{}) {
    kept.visible = visible;
    object = std::move(kept);
  } else {
    // Preserve T_parent * T_operand exactly, even when flattening would introduce shear.
    scene::Object group;
    group.name = kept.name + " (unwrapped)";
    group.transform = outer_transform;
    group.visible = visible;
    group.content = scene::GroupSpec{.children = {std::move(kept)}};
    object = std::move(group);
  }
  return true;
}

auto DrawPatternEditor(scene::Pattern& pattern, int depth) -> bool {  // NOLINT(misc-no-recursion)
  bool changed = false;
  static constexpr std::array kPatternNames = {
      "Solid", "Stripe", "Gradient", "Ring", "Checkers", "Radial Gradient", "UV Checkers", "Blended", "Perturbed"};
  int pattern_index = PatternTypeIndex(pattern.type);
  ImGui::PushID(depth);
  if (ImGui::Combo("Type", &pattern_index, kPatternNames.data(), static_cast<int>(kPatternNames.size()))) {
    pattern.type = PatternTypeFromIndex(pattern_index);
    EnsurePatternChildren(pattern);
    changed = true;
  }

  if (pattern.type == scene::PatternType::kSolid) {
    changed |= ColorE3("Color", pattern.color);
  } else {
    if (pattern.type == scene::PatternType::kPerturbed) {
      changed |= SliderF("Perturb Scale", &pattern.perturb_scale, 0.0F, 2.0F);
    }
    if (pattern.type == scene::PatternType::kUvCheckers) {
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
      const std::string label = pattern.type == scene::PatternType::kPerturbed ? "Source" : "Child " + std::to_string(i + 1);
      if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= DrawPatternEditor(pattern.children[i], depth + 1 + static_cast<int>(i));
        ImGui::TreePop();
      }
    }
  }
  ImGui::PopID();
  return changed;
}

auto DrawMaterialEditor(scene::Material& material) -> bool {
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
      material.pattern = scene::Pattern{};
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

auto DrawObjectEditor(scene::Object& object, const bool allow_hide) -> bool {
  bool changed = false;
  if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    changed |= DragF3("Translate", object.transform.translate, 0.1F, -50.0F, 50.0F);
    changed |= DragF3("Scale", object.transform.scale, 0.01F, 0.01F, 20.0F);
    changed |= DragF3("Rotate (deg)", object.transform.rotate_deg, 1.0F, -360.0F, 360.0F);
    ImGui::BeginDisabled(!allow_hide);
    changed |= ImGui::Checkbox("Visible", &object.visible);
    ImGui::EndDisabled();
    ImGui::TreePop();
  }
  if (object.IsGroup()) {
    ImGui::Text("Children: %zu", object.Group().children.size());
    return changed;
  }

  if (ImGui::Button("Wrap in CSG")) {
    const auto name = object.name;
    object = scene::MakeCsg(std::move(object));
    object.name = name + " CSG";
    return true;
  }
  if (object.IsCsg()) {
    if (ImGui::Button("Unwrap")) {
      ImGui::OpenPopup("UnwrapCsg");
    }
    std::optional<std::size_t> keep_operand;
    if (ImGui::BeginPopup("UnwrapCsg")) {
      ImGui::TextDisabled("Keep one operand; discard the other");
      if (ImGui::MenuItem("Keep Left")) {
        keep_operand = 0;
      }
      if (ImGui::MenuItem("Keep Right")) {
        keep_operand = 1;
      }
      ImGui::EndPopup();
    }
    if (keep_operand.has_value()) {
      return UnwrapCsg(object, *keep_operand);
    }
    auto& csg = object.Csg();
    static constexpr std::array kOperations = {"Union", "Intersection", "Difference"};
    int operation = static_cast<int>(csg.operation);
    if (ImGui::Combo("Operation", &operation, kOperations.data(), static_cast<int>(kOperations.size()))) {
      csg.operation = static_cast<scene::CsgOperation>(operation);
      changed = true;
    }
    changed |= ImGui::Checkbox("Cast Shadow", &csg.cast_shadow);
    ImGui::TextWrapped("Expand this CSG in the object tree to select and edit its left/right operands.");
    bool override_material = csg.material.has_value();
    if (ImGui::Checkbox("Override operand materials", &override_material)) {
      csg.material = override_material ? std::optional{scene::Material{}} : std::nullopt;
      changed = true;
    }
    if (csg.material.has_value() && ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
      changed |= DrawMaterialEditor(*csg.material);
      ImGui::TreePop();
    }
    return changed;
  }

  auto& shape = object.Shape();
  if (shape.type == scene::ObjectType::kObj) {
    std::array<char, 1024> source_buffer{};
    std::memcpy(source_buffer.data(), shape.source.data(),
                std::min(shape.source.size(), source_buffer.size() - 1));
    if (ImGui::InputText("OBJ source", source_buffer.data(), source_buffer.size())) {
      shape.source = source_buffer.data();
    }
    changed |= ImGui::IsItemDeactivatedAfterEdit();
    const auto dialog_key = "ObjSourceDialog" + std::to_string(ImGui::GetID("OBJ source"));
    if (ImGui::Button("Browse OBJ")) {
      ImGuiFileDialog::Instance()->OpenDialog(dialog_key, "Select OBJ Model", ".obj",
                                             shape.source.empty() ? std::filesystem::path{RAY_TRACER_ASSET_DIR}
                                                                  : std::filesystem::path{shape.source});
    }
    if (ImGuiFileDialog::Instance()->Display(dialog_key) && ImGuiFileDialog::Instance()->IsOk()) {
      shape.source = ImGuiFileDialog::Instance()->GetFilePathName();
      changed = true;
    }
  }
  const auto current_name = shape_catalog::DisplayName(shape.type);
  if (ImGui::BeginCombo("Shape", current_name.c_str())) {
    for (const auto& registration : shape_catalog::All()) {
      const bool selected = registration.type == shape.type;
      const auto display_name = shape_catalog::DisplayName(registration.serialized_name);
      if (ImGui::Selectable(display_name.c_str(), selected)) {
        shape.type = registration.type;
        changed = true;
      }
      if (selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (shape_catalog::Find(shape.type).supports_axial_bounds &&
      ImGui::TreeNodeEx("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
    changed |= DrawAxialBoundsEditor(shape.axial_bounds);
    ImGui::TreePop();
  }
  if ((shape.type == scene::ObjectType::kTriangle || shape.type == scene::ObjectType::kSmoothTriangle) &&
      ImGui::TreeNodeEx("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
    for (std::size_t index = 0; index < shape.vertices.size(); ++index) {
      const auto label = "Vertex " + std::to_string(index + 1);
      changed |= DragF3(label.c_str(), shape.vertices[index], 0.05F, 0.0F, 0.0F);
    }
    if (shape.type == scene::ObjectType::kSmoothTriangle) {
      for (std::size_t index = 0; index < shape.normals.size(); ++index) {
        const auto label = "Normal " + std::to_string(index + 1);
        changed |= DragF3(label.c_str(), shape.normals[index], 0.05F, 0.0F, 0.0F);
      }
    }
    ImGui::TreePop();
  }
  changed |= ImGui::Checkbox("Cast Shadow", &shape.cast_shadow);
  if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
    changed |= DrawMaterialEditor(shape.material);
    ImGui::TreePop();
  }
  return changed;
}

}  // namespace ray_tracer::scene_editor
