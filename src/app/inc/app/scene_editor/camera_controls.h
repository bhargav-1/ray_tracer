#pragma once

#include "app/scene/scene.h"

namespace ray_tracer::scene_editor {

auto OrbitCamera(scene::Camera& camera, float delta_x, float delta_y) -> void;
auto PanCamera(scene::Camera& camera, float delta_x, float delta_y) -> void;
auto ZoomCamera(scene::Camera& camera, float delta) -> void;

}  // namespace ray_tracer::scene_editor
