#include <app/scene/scene_yaml.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#include "app/scene/scene.h"

namespace ray_tracer {
namespace {

constexpr auto kMinimalSceneYaml = R"yaml(
version: 1
camera:
  from: [0, 1.5, -5]
  to: [0, 1, 0]
  up: [0, 1, 0]
  fov_deg: 60
  resolution: [320, 200]
lights: []
objects: []
)yaml";

constexpr auto kFullSceneYaml = R"yaml(
version: 1
camera:
  from: [0, 1.5, -5]
  to: [0, 1, 0]
  up: [0, 1, 0]
  fov_deg: 60
  resolution: [320, 200]
lights:
  - visible: false
    position: [-10, 10, -10]
    color: [1, 1, 1]
objects:
  - name: floor
    type: plane
    visible: false
    cast_shadow: false
    transform:
      translate: [1, 2, 3]
      rotate_deg: [10, 20, 30]
      scale: [4, 5, 6]
    material:
      color: [0.8, 0.7, 0.6]
      ambient: 0.11
      diffuse: 0.22
      specular: 0.33
      shininess: 44
      reflective: 0.55
      transparency: 0.66
      refractive_index: 1.33
      pattern:
        type: perturbed
        scale: 0.6
        source:
          type: stripe
          children:
            - type: solid
              color: [1, 0, 0]
            - type: solid
              color: [0, 0, 1]
          transform:
            translate: [0, 0, 0]
            rotate_deg: [0, 0, 0]
            scale: [0.5, 0.5, 0.5]
        transform:
          translate: [0, 0, 0]
          rotate_deg: [0, 0, 0]
          scale: [1, 1, 1]
)yaml";

}  // namespace

SCENARIO("scene yaml: parses minimal valid scene YAML") {
  const auto result = scene_io::ParseSceneYaml(kMinimalSceneYaml);

  REQUIRE(result.ok);
  if (!result.scene.camera.has_value()) {
    FAIL("expected parsed scene camera");
  }
  CHECK(result.scene.camera->resolution == scene::Resolution{320, 200});
  CHECK(result.scene.lights.empty());
  CHECK(result.scene.objects.empty());
}

SCENARIO("scene yaml: parses camera, resolution, light, object, material, transform, and recursive pattern") {
  const auto result = scene_io::ParseSceneYaml(kFullSceneYaml);

  REQUIRE(result.ok);
  REQUIRE(result.scene.camera.has_value());
  REQUIRE(result.scene.lights.size() == 1);
  REQUIRE(result.scene.objects.size() == 1);
  const auto& object = result.scene.objects[0];
  CHECK_FALSE(result.scene.lights[0].visible);
  CHECK(object.name == "floor");
  CHECK(object.type == scene::ObjectType::kPlane);
  CHECK_FALSE(object.visible);
  CHECK_FALSE(object.cast_shadow);
  CHECK(object.transform.translate == scene::Vec3{1.0F, 2.0F, 3.0F});
  CHECK(object.transform.rotate_deg == scene::Vec3{10.0F, 20.0F, 30.0F});
  CHECK(object.transform.scale == scene::Vec3{4.0F, 5.0F, 6.0F});
  CHECK(object.material.color == scene::ColorRgb{0.8F, 0.7F, 0.6F});
  if (!object.material.pattern.has_value()) {
    FAIL("expected parsed object pattern");
  }
  const auto& pattern = *object.material.pattern;
  CHECK(pattern.type == scene::PatternType::kPerturbed);
  REQUIRE(pattern.children.size() == 1);
  CHECK(pattern.children[0].type == scene::PatternType::kStripe);
}

SCENARIO("scene yaml: parses and serializes cube objects") {
  auto cube_yaml = std::string{kFullSceneYaml};
  cube_yaml.replace(cube_yaml.find("type: plane"), std::string{"type: plane"}.size(), "type: cube");

  const auto parsed = scene_io::ParseSceneYaml(cube_yaml);

  REQUIRE(parsed.ok);
  REQUIRE(parsed.scene.objects.size() == 1);
  CHECK(parsed.scene.objects[0].type == scene::ObjectType::kCube);

  const auto serialized = scene_io::SerializeSceneYaml(parsed.scene);
  CHECK(serialized.find("type: cube") != std::string::npos);

  const auto reparsed = scene_io::ParseSceneYaml(serialized);
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == parsed.scene);
}

SCENARIO("scene yaml: rejects invalid scenes with useful errors") {
  CHECK_FALSE(scene_io::ParseSceneYaml("camera: {}\nlights: []\nobjects: []\n").ok);
  CHECK_FALSE(scene_io::ParseSceneYaml(std::string{kMinimalSceneYaml} + "\nextra: 1\n").ok);

  auto unknown_object = std::string{kFullSceneYaml};
  unknown_object.replace(unknown_object.find("type: plane"), std::string{"type: plane"}.size(), "type: cone");
  const auto object_result = scene_io::ParseSceneYaml(unknown_object);
  CHECK_FALSE(object_result.ok);
  CHECK_FALSE(object_result.error.empty());

  auto unknown_pattern = std::string{kFullSceneYaml};
  unknown_pattern.replace(
      unknown_pattern.find("type: stripe"), std::string{"type: stripe"}.size(), "type: noise");
  const auto pattern_result = scene_io::ParseSceneYaml(unknown_pattern);
  CHECK_FALSE(pattern_result.ok);
  CHECK_FALSE(pattern_result.error.empty());

  auto malformed_vector = std::string{kMinimalSceneYaml};
  malformed_vector.replace(malformed_vector.find("from: [0, 1.5, -5]"), 19, "from: [1, 2]");
  const auto vector_result = scene_io::ParseSceneYaml(malformed_vector);
  CHECK_FALSE(vector_result.ok);
  CHECK_FALSE(vector_result.error.empty());
}

SCENARIO("scene yaml: serializes and round trips scenes") {
  const auto parsed = scene_io::ParseSceneYaml(kFullSceneYaml);
  REQUIRE(parsed.ok);

  const auto serialized = scene_io::SerializeSceneYaml(parsed.scene);
  CHECK(serialized.find("version") != std::string::npos);
  CHECK(serialized.find("pattern") != std::string::npos);

  const auto reparsed = scene_io::ParseSceneYaml(serialized);
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == parsed.scene);
}

SCENARIO("scene yaml: saves readable yaml files") {
  const auto parsed = scene_io::ParseSceneYaml(kFullSceneYaml);
  REQUIRE(parsed.ok);
  const auto path = std::filesystem::temp_directory_path() / "ray_tracer_scene_yaml_io_test.yaml";

  const auto saved = scene_io::SaveSceneYaml(parsed.scene, path);
  REQUIRE(saved.ok);
  const auto loaded = scene_io::LoadSceneYaml(path);
  REQUIRE(loaded.ok);
  CHECK(loaded.scene == parsed.scene);
  std::filesystem::remove(path);
}

}  // namespace ray_tracer
