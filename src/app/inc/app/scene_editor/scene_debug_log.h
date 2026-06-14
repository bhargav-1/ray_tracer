#pragma once

#include <cstddef>
#include <cstdint>

#include "app/camera.h"
#include "app/scene/scene.h"
#include "app/scene/scene_compiler.h"

namespace ray_tracer::scene_editor {

[[nodiscard]] auto CountVisibleObjects(const scene::Scene& scene) -> std::size_t;
[[nodiscard]] auto SceneFingerprint(const scene::Scene& scene) -> std::uint64_t;

auto LogAuthoringSceneSnapshot(const char* mode, const scene::Scene& scene) -> void;
auto LogRuntimeSceneSnapshot(const char* mode,
                             int render_id,
                             const scene_runtime::CompiledScene& compiled_scene) -> void;
auto LogRayFingerprint(const char* label, int render_id, Camera camera) -> void;

}  // namespace ray_tracer::scene_editor
