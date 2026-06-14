#pragma once

#include "app/scene/scene.h"

namespace ray_tracer::scene_editor {

auto DrawPatternEditor(scene::Pattern& pattern, int depth = 0) -> bool;
auto DrawMaterialEditor(scene::Material& material) -> bool;
auto DrawObjectEditor(scene::Object& object) -> bool;

}  // namespace ray_tracer::scene_editor
