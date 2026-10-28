#include <app/scene/scene_yaml.h>
#include <app/scene_editor/scene_editor_app.h>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include "app/asset_directory.h"
#include "app/scene/scene.h"
#include "app/scene/scene_compiler.h"

namespace ray_tracer {
namespace {

struct TemporaryInstall {
  std::filesystem::path root =
      std::filesystem::temp_directory_path() /
      ("ray_tracer_assets_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));

  TemporaryInstall() {
    std::filesystem::create_directories(root / "bin");
    std::ofstream(root / "bin" / "raytracerchallenge").close();
  }

  ~TemporaryInstall() {
    std::error_code error;
    std::filesystem::remove_all(root, error);
  }
};

auto ValidScene() -> scene::Scene {
  scene::Scene scene;
  scene.camera = scene::Camera{};
  scene.lights.push_back(scene::Light{});
  scene::Object sphere;
  sphere.name = "sphere";
  sphere.Shape().type = scene::ObjectType::kSphere;
  scene.objects.push_back(sphere);
  return scene;
}

}  // namespace

SCENARIO("asset lookup: development builds and missing executables use checkout assets") {
  CHECK(AssetDirectory() == std::filesystem::path{RAY_TRACER_ASSET_DIR});
  CHECK(AssetDirectory("/nonexistent/ray_tracer/raytracerchallenge") == std::filesystem::path{RAY_TRACER_ASSET_DIR});
}

SCENARIO("asset lookup: relocated installations load bundled scenes and relative OBJ models") {
  const TemporaryInstall install;
  const auto assets = install.root / RAY_TRACER_INSTALL_ASSET_DIR;
  std::filesystem::create_directories(assets.parent_path());
  std::filesystem::copy(RAY_TRACER_ASSET_DIR, assets, std::filesystem::copy_options::recursive);
  REQUIRE(AssetDirectory(install.root / "bin" / "raytracerchallenge") == assets);

  const auto default_scene = scene_io::LoadSceneYaml(assets / "scenes" / "default.yaml");
  REQUIRE(default_scene.ok);
  CHECK(scene_runtime::CompileScene(default_scene.scene).ok);
  const auto mesh_scene = scene_io::LoadSceneYaml(assets / "scenes" / "teapot_comparision_chapter15.yaml");
  REQUIRE(mesh_scene.ok);
  CHECK(scene_runtime::CompileScene(mesh_scene.scene).ok);
}

SCENARIO("scene editor app: selected viewport is disabled by default") {
  const scene_editor::SceneEditorApp app;
  CHECK_FALSE(app.State().show_selected_viewport);
}

SCENARIO("scene editor app: load replaces current scene only on success") {
  scene_editor::SceneEditorApp app;
  REQUIRE(app.SetCurrentScene(ValidScene()));
  const auto original_count = app.CurrentScene().objects.size();

  auto replacement = ValidScene();
  scene::Object floor;
  floor.name = "floor";
  floor.Shape().type = scene::ObjectType::kPlane;
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
