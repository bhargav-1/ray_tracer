#include <app/scene/scene_yaml.h>
#include <app/scene/scene_compiler.h>
#include <app/utilities.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <numbers>

#include "app/cube.h"
#include "app/group.h"
#include "app/matrix.h"
#include "app/scene/scene.h"

namespace ray_tracer {
namespace {

auto Solid(scene::ColorRgb color) -> scene::Pattern {
  scene::Pattern pattern;
  pattern.type = scene::PatternType::kSolid;
  pattern.color = color;
  return pattern;
}

auto MakeCompiledSceneModel() -> scene::Scene {
  scene::Scene scene;
  scene.camera = scene::Camera{
      .from = {0.0F, 1.5F, -5.0F},
      .to = {0.0F, 1.0F, 0.0F},
      .up = {0.0F, 1.0F, 0.0F},
      .fov_deg = 60.0F,
      .resolution = {320, 200}};
  scene.lights.push_back({.position = {-10.0F, 10.0F, -10.0F}, .color = {1.0F, 1.0F, 1.0F}});
  scene.lights.push_back(
      {.position = {10.0F, 10.0F, -10.0F}, .color = {1.0F, 1.0F, 1.0F}, .visible = false});

  scene::Pattern stripe;
  stripe.type = scene::PatternType::kStripe;
  stripe.children = {Solid({1.0F, 0.0F, 0.0F}), Solid({0.0F, 0.0F, 1.0F})};

  scene::Pattern perturbed;
  perturbed.type = scene::PatternType::kPerturbed;
  perturbed.perturb_scale = 0.6F;
  perturbed.children = {stripe};

  scene::Object plane;
  plane.name = "floor";
  plane.Shape().type = scene::ObjectType::kPlane;
  plane.Shape().cast_shadow = false;
  plane.transform.translate = {0.0F, -1.0F, 0.0F};
  plane.Shape().material.specular = 0.0F;
  plane.Shape().material.pattern = perturbed;
  scene.objects.push_back(plane);

  scene::Object sphere;
  sphere.name = "sphere";
  sphere.Shape().type = scene::ObjectType::kSphere;
  sphere.transform.translate = {1.0F, 2.0F, 3.0F};
  sphere.transform.rotate_deg = {0.0F, 90.0F, 0.0F};
  sphere.transform.scale = {0.5F, 0.5F, 0.5F};
  sphere.Shape().material.color = {0.8F, 1.0F, 0.6F};
  sphere.Shape().material.diffuse = 0.7F;
  sphere.Shape().material.specular = 0.2F;
  sphere.Shape().material.reflective = 0.1F;
  scene.objects.push_back(sphere);

  scene::Object cube;
  cube.name = "cube";
  cube.Shape().type = scene::ObjectType::kCube;
  cube.transform.translate = {-1.0F, 0.0F, 0.0F};
  cube.transform.scale = {0.25F, 0.25F, 0.25F};
  scene.objects.push_back(cube);

  scene::Object hidden;
  hidden.name = "hidden";
  hidden.Shape().type = scene::ObjectType::kSphere;
  hidden.visible = false;
  scene.objects.push_back(hidden);

  return scene;
}

}  // namespace

SCENARIO("scene compiler: compiles camera, lights, objects, transforms, materials, shadows, and patterns") {
  const auto scene = MakeCompiledSceneModel();
  const auto result = scene_runtime::CompileScene(scene);

  REQUIRE(result.ok);
  if (!result.compiled.has_value()) {
    FAIL("expected compiled scene");
  }
  const auto& compiled = *result.compiled;
  CHECK(compiled.camera.GetHorizontalSize() == 320);
  CHECK(compiled.camera.GetVerticalSize() == 200);
  CHECK(utilities::Equal(compiled.camera.GetFieldOfView(), std::numbers::pi / 3.0));
  REQUIRE(compiled.world.Lights().size() == 1);
  REQUIRE(compiled.world.Objects().size() == 3);
  CHECK_FALSE(compiled.world.Objects()[0]->GetCastShadow());
  CHECK(compiled.world.Objects()[0]->GetMaterial().HasPattern());
  CHECK(compiled.world.Objects()[1]->GetTransform() ==
        ray_tracer::Matrix<4, 4>::Translation(1, 2, 3) *
            ray_tracer::Matrix<4, 4>::RotationY(std::numbers::pi / 2.0) *
            ray_tracer::Matrix<4, 4>::Scaling(0.5, 0.5, 0.5));
  CHECK(compiled.world.Objects()[1]->GetMaterial().GetColor() == ray_tracer::Color(0.8, 1.0, 0.6));
  CHECK(utilities::Equal(compiled.world.Objects()[1]->GetMaterial().GetReflective(), 0.1));
  CHECK(dynamic_cast<const ray_tracer::Cube*>(compiled.world.Objects()[2].get()) != nullptr);
  CHECK(compiled.world.Objects()[2]->GetTransform() ==
        ray_tracer::Matrix<4, 4>::Translation(-1, 0, 0) *
            ray_tracer::Matrix<4, 4>::Scaling(0.25, 0.25, 0.25));
}

SCENARIO("scene compiler: returns useful errors") {
  const scene::Scene scene;
  const auto result = scene_runtime::CompileScene(scene);
  CHECK_FALSE(result.ok);
  CHECK_FALSE(result.error.empty());
}

SCENARIO("scene compiler: default asset loads and compiles") {
  const auto path = std::filesystem::path{RAY_TRACER_ASSET_DIR} / "scenes" / "default.yaml";
  const auto loaded = scene_io::LoadSceneYaml(path);
  REQUIRE(loaded.ok);
  REQUIRE(loaded.scene.camera.has_value());
  CHECK(!loaded.scene.lights.empty());
  CHECK(loaded.scene.objects.size() >= 2);
  CHECK(loaded.scene.objects[0].Shape().type == scene::ObjectType::kPlane);
  CHECK(loaded.scene.objects[1].Shape().type == scene::ObjectType::kSphere);

  const auto compiled = scene_runtime::CompileScene(loaded.scene);
  REQUIRE(compiled.ok);
}

SCENARIO("scene compiler: compiles recursive groups") {
  auto scene = MakeCompiledSceneModel();
  scene.objects.clear();
  scene::Object group;
  group.name = "scaled group";
  group.content = scene::GroupSpec{};
  group.transform.scale = {2.0F, 2.0F, 2.0F};
  scene::Object child;
  child.name = "child cube";
  child.Shape().type = scene::ObjectType::kCube;
  child.transform.translate = {1.0F, 0.0F, 0.0F};
  group.Group().children.push_back(child);
  scene.objects.push_back(group);

  const auto compiled = scene_runtime::CompileScene(scene);

  REQUIRE(compiled.ok);
  if (!compiled.compiled.has_value()) {
    FAIL("expected compiled scene");
  }
  const auto& runtime = *compiled.compiled;
  REQUIRE(runtime.world.Objects().size() == 1);
  const auto runtime_group = std::dynamic_pointer_cast<Group>(runtime.world.Objects()[0]);
  REQUIRE(runtime_group != nullptr);
  REQUIRE(runtime_group->GetChildren().size() == 1);
  CHECK(dynamic_cast<const Cube*>(runtime_group->GetChildren()[0].get()) != nullptr);
  CHECK(runtime_group->GetChildren()[0]->GetParentGroup().lock() == runtime_group);
}

}  // namespace ray_tracer
