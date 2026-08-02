#include <app/scene_editor/scene_editor_app.h>
#include <app/scene/scene_yaml.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "app/scene/scene.h"

namespace ray_tracer {
namespace {

auto ValidScene() -> scene::Scene {
  scene::Scene scene;
  scene.camera = scene::Camera{};
  scene.lights.push_back(scene::Light{});
  scene::Object sphere;
  sphere.name = "sphere";
  sphere.type = scene::ObjectType::kSphere;
  scene.objects.push_back(sphere);
  return scene;
}

}  // namespace

SCENARIO("scene editor app: viewport overlays are disabled by default") {
  const scene_editor::SceneEditorApp app;
  CHECK_FALSE(app.State().show_viewport_overlays);
}

SCENARIO("scene editor app: load replaces current scene only on success") {
  scene_editor::SceneEditorApp app;
  REQUIRE(app.SetCurrentScene(ValidScene()));
  const auto original_count = app.CurrentScene().objects.size();

  auto replacement = ValidScene();
  scene::Object floor;
  floor.name = "floor";
  floor.type = scene::ObjectType::kPlane;
  replacement.objects.push_back(floor);
  const auto path = std::filesystem::temp_directory_path() / "ray_tracer_scene_editor_load.yaml";
  REQUIRE(scene_io::SaveSceneYaml(replacement, path).ok);

  REQUIRE(app.LoadScene(path));
  CHECK(app.CurrentScene().objects.size() == 2);

  CHECK_FALSE(app.LoadScene(path.parent_path() / "missing_scene.yaml"));
  CHECK(app.CurrentScene().objects.size() == 2);
  CHECK(app.CurrentScene().objects.size() != original_count);
  std::filesystem::remove(path);
}

SCENARIO("scene editor app: save writes current scene") {
  scene_editor::SceneEditorApp app;
  REQUIRE(app.SetCurrentScene(ValidScene()));
  const auto path = std::filesystem::temp_directory_path() / "ray_tracer_scene_editor_save.yaml";

  REQUIRE(app.SaveScene(path));
  const auto loaded = scene_io::LoadSceneYaml(path);
  REQUIRE(loaded.ok);
  CHECK(loaded.scene == app.CurrentScene());
  std::filesystem::remove(path);
}

}  // namespace ray_tracer
