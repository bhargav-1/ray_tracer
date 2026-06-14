#include "app/scene_editor/camera_controls.h"

#include <algorithm>
#include <cmath>

#include "app/scene/scene.h"

namespace ray_tracer::scene_editor {

auto OrbitCamera(scene::Camera& camera, float delta_x, float delta_y) -> void {
  const float forward_x = camera.from.x - camera.to.x;
  const float forward_y = camera.from.y - camera.to.y;
  const float forward_z = camera.from.z - camera.to.z;
  const float radius = std::sqrt((forward_x * forward_x) + (forward_y * forward_y) + (forward_z * forward_z));
  if (radius < 0.001F) {
    return;
  }
  float theta = std::atan2(forward_x, forward_z);
  float phi = std::asin(std::clamp(forward_y / radius, -1.0F, 1.0F));
  theta -= delta_x * 0.01F;
  phi = std::clamp(phi + (delta_y * 0.01F), -1.5F, 1.5F);
  camera.from.x = camera.to.x + radius * std::sin(theta) * std::cos(phi);
  camera.from.y = camera.to.y + radius * std::sin(phi);
  camera.from.z = camera.to.z + radius * std::cos(theta) * std::cos(phi);
}

auto PanCamera(scene::Camera& camera, float delta_x, float delta_y) -> void {
  float forward_x = camera.to.x - camera.from.x;
  float forward_y = camera.to.y - camera.from.y;
  float forward_z = camera.to.z - camera.from.z;
  const float length = std::sqrt((forward_x * forward_x) + (forward_y * forward_y) + (forward_z * forward_z));
  if (length < 0.001F) {
    return;
  }
  forward_x /= length;
  forward_y /= length;
  forward_z /= length;
  float right_x = forward_z;
  float right_z = -forward_x;
  const float right_length = std::sqrt((right_x * right_x) + (right_z * right_z));
  if (right_length < 0.001F) {
    return;
  }
  right_x /= right_length;
  right_z /= right_length;
  float up_x = -right_z * forward_y;
  float up_y = (right_z * forward_x) - (right_x * forward_z);
  float up_z = right_x * forward_y;
  const float up_length = std::sqrt((up_x * up_x) + (up_y * up_y) + (up_z * up_z));
  if (up_length < 0.001F) {
    return;
  }
  up_x /= up_length;
  up_y /= up_length;
  up_z /= up_length;

  const float speed = 0.005F * length;
  const float move_x = ((-delta_x * right_x) + (delta_y * up_x)) * speed;
  const float move_y = (delta_y * up_y) * speed;
  const float move_z = ((-delta_x * right_z) + (delta_y * up_z)) * speed;
  camera.from.x += move_x;
  camera.from.y += move_y;
  camera.from.z += move_z;
  camera.to.x += move_x;
  camera.to.y += move_y;
  camera.to.z += move_z;
}

auto ZoomCamera(scene::Camera& camera, float delta) -> void {
  const float forward_x = camera.from.x - camera.to.x;
  const float forward_y = camera.from.y - camera.to.y;
  const float forward_z = camera.from.z - camera.to.z;
  const float radius = std::sqrt((forward_x * forward_x) + (forward_y * forward_y) + (forward_z * forward_z));
  if (radius < 0.001F) {
    return;
  }
  const float new_radius = std::max(0.5F, radius * std::clamp(1.0F - (delta * 0.1F), 0.1F, 3.0F));
  const float scale = new_radius / radius;
  camera.from.x = camera.to.x + forward_x * scale;
  camera.from.y = camera.to.y + forward_y * scale;
  camera.from.z = camera.to.z + forward_z * scale;
}

}  // namespace ray_tracer::scene_editor
