#include <app/scene_editor/scene_renderer.h>
#include <app/scene/scene_compiler.h>
#include <catch2/catch_test_macros.hpp>

#include "app/scene/scene.h"

namespace ray_tracer {

SCENARIO("scene renderer: converts CPU colors to RGB8") {
  scene_editor::RenderPixels image;
  image.width = 2;
  image.height = 1;
  image.pixels = {Color(-1.0, 0.5, 2.0), Color(0.25, 0.0, 1.0)};

  const auto rgb = scene_editor::SceneRenderer::ToRgb8(image);

  REQUIRE(rgb.size() == 6);
  CHECK(rgb[0] == 0);
  CHECK(rgb[1] == 127);
  CHECK(rgb[2] == 255);
  CHECK(rgb[3] == 63);
  CHECK(rgb[4] == 0);
  CHECK(rgb[5] == 255);
}

SCENARIO("scene renderer: renders fast low resolution previews") {
  scene::Scene scene;
  scene.camera = scene::Camera{.resolution = {80, 40}};
  scene.lights.push_back(scene::Light{});

  scene::Pattern pattern;
  pattern.type = scene::PatternType::kStripe;
  scene::Pattern red;
  red.type = scene::PatternType::kSolid;
  red.color = {1.0F, 0.0F, 0.0F};
  scene::Pattern white;
  white.type = scene::PatternType::kSolid;
  white.color = {1.0F, 1.0F, 1.0F};
  pattern.children = {red, white};

  scene::Object floor;
  floor.Shape().type = scene::ObjectType::kPlane;
  floor.Shape().material.reflective = 0.5F;
  floor.Shape().material.pattern = pattern;
  scene.objects.push_back(floor);

  auto compiled = scene_runtime::CompileScene(scene);
  REQUIRE(compiled.ok);
  if (!compiled.compiled.has_value()) {
    FAIL("expected compiled scene");
  }

  const auto preview = scene_editor::SceneRenderer::Render(*compiled.compiled, true, nullptr, nullptr);

  CHECK(preview.width == 40);
  CHECK(preview.height == 20);
}

}  // namespace ray_tracer
