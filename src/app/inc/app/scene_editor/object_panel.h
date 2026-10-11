#pragma once

#include <cstddef>

#include "app/scene/scene.h"

namespace ray_tracer::scene_editor {

auto DrawPatternEditor(scene::Pattern& pattern, int depth = 0) -> bool;
auto DrawMaterialEditor(scene::Material& material) -> bool;
auto DrawObjectEditor(scene::Object& object, bool allow_hide = true) -> bool;
auto UnwrapCsg(scene::Object& object, std::size_t operand_index) -> bool;

}  // namespace ray_tracer::scene_editor
