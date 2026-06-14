#pragma once

#include "app/scene/scene.h"

namespace ray_tracer::scene_editor {

auto SliderF(const char* label, float* value, float low, float high) -> bool;
auto DragF3(const char* label, scene::Vec3& value, float speed, float low, float high) -> bool;
auto ColorE3(const char* label, scene::ColorRgb& value) -> bool;

}  // namespace ray_tracer::scene_editor
