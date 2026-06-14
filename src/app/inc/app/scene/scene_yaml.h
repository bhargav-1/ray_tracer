#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "app/scene/scene.h"

namespace ray_tracer::scene_io {

struct LoadSceneResult {
  bool ok{false};
  scene::Scene scene{};
  std::string error;
};

struct SaveSceneResult {
  bool ok{false};
  std::string error;
};

auto ParseSceneYaml(std::string_view yaml_text) -> LoadSceneResult;
auto LoadSceneYaml(const std::filesystem::path& path) -> LoadSceneResult;
auto SerializeSceneYaml(const scene::Scene& scene) -> std::string;
auto SaveSceneYaml(const scene::Scene& scene, const std::filesystem::path& path) -> SaveSceneResult;

}  // namespace ray_tracer::scene_io
