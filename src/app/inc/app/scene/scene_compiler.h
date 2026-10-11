#pragma once

#include <optional>
#include <string>

#include "app/camera.h"
#include "app/scene/scene.h"
#include "app/world.h"

namespace ray_tracer::scene_runtime {

struct CompiledScene {
  ray_tracer::World world;
  ray_tracer::Camera camera;
};

struct CompileSceneResult {
  bool ok{false};
  std::optional<CompiledScene> compiled;
  std::string error;
};

auto CompileScene(const scene::Scene& scene) -> CompileSceneResult;
auto CompileCamera(const scene::Camera& camera) -> ray_tracer::Camera;

}  // namespace ray_tracer::scene_runtime
