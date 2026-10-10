#include <app/scene/scene_yaml.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_message.hpp>

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
  const auto& shape = object.Shape();
  CHECK_FALSE(result.scene.lights[0].visible);
  CHECK(object.name == "floor");
  CHECK(shape.type == scene::ObjectType::kPlane);
  CHECK_FALSE(object.visible);
  CHECK_FALSE(shape.cast_shadow);
  CHECK(object.transform.translate == scene::Vec3{1.0F, 2.0F, 3.0F});
  CHECK(object.transform.rotate_deg == scene::Vec3{10.0F, 20.0F, 30.0F});
  CHECK(object.transform.scale == scene::Vec3{4.0F, 5.0F, 6.0F});
  CHECK(shape.material.color == scene::ColorRgb{0.8F, 0.7F, 0.6F});
  if (!shape.material.pattern.has_value()) {
    FAIL("expected parsed object pattern");
  }
  const auto& pattern = *shape.material.pattern;
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
  CHECK(parsed.scene.objects[0].Shape().type == scene::ObjectType::kCube);

  const auto serialized = scene_io::SerializeSceneYaml(parsed.scene);
  CHECK(serialized.find("type: cube") != std::string::npos);

  const auto reparsed = scene_io::ParseSceneYaml(serialized);
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == parsed.scene);
}

SCENARIO("scene yaml: parses and serializes triangle objects") {
  auto triangle_yaml = std::string{kFullSceneYaml};
  triangle_yaml.replace(triangle_yaml.find("type: plane"), std::string{"type: plane"}.size(), "type: triangle");

  const auto parsed = scene_io::ParseSceneYaml(triangle_yaml);
  REQUIRE(parsed.ok);
  REQUIRE(parsed.scene.objects.size() == 1);
  CHECK(parsed.scene.objects[0].Shape().type == scene::ObjectType::kTriangle);
  CHECK(parsed.scene.objects[0].Shape().vertices[0] == scene::Vec3{0, 1, 0});

  const auto serialized = scene_io::SerializeSceneYaml(parsed.scene);
  CHECK(serialized.find("type: triangle") != std::string::npos);
  const auto reparsed = scene_io::ParseSceneYaml(serialized);
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == parsed.scene);
}

SCENARIO("scene yaml: preserves custom triangle vertices and rejects malformed geometry") {
  auto yaml = std::string{kFullSceneYaml};
  yaml.replace(yaml.find("type: plane"), std::string{"type: plane"}.size(),
               "type: triangle\n    vertices: [[-2, 0, 2], [2, 0, 2], [0, 3, 2]]");
  const auto parsed = scene_io::ParseSceneYaml(yaml);
  REQUIRE(parsed.ok);
  const auto& vertices = parsed.scene.objects[0].Shape().vertices;
  CHECK(vertices[0] == scene::Vec3{-2, 0, 2});
  CHECK(vertices[1] == scene::Vec3{2, 0, 2});
  CHECK(vertices[2] == scene::Vec3{0, 3, 2});
  const auto reparsed = scene_io::ParseSceneYaml(scene_io::SerializeSceneYaml(parsed.scene));
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == parsed.scene);

  auto malformed = yaml;
  const auto valid_vertices = std::string{"[[-2, 0, 2], [2, 0, 2], [0, 3, 2]]"};
  malformed.replace(malformed.find(valid_vertices), valid_vertices.size(), "[[0, 0, 0], [1, 0, 0]]");
  CHECK_FALSE(scene_io::ParseSceneYaml(malformed).ok);
  auto unsupported = yaml;
  unsupported.replace(unsupported.find("type: triangle"), std::string{"type: triangle"}.size(), "type: cube");
  CHECK_FALSE(scene_io::ParseSceneYaml(unsupported).ok);
}

SCENARIO("scene yaml: parses and serializes nested groups") {
  const auto group_yaml = std::string{kMinimalSceneYaml}.replace(
      std::string{kMinimalSceneYaml}.find("objects: []"), std::string{"objects: []"}.size(), R"yaml(objects:
  - name: furniture
    type: group
    visible: true
    transform:
      translate: [1, 0, 0]
      rotate_deg: [0, 45, 0]
      scale: [1, 1, 1]
    children:
      - name: tabletop
        type: cube
        visible: true
        cast_shadow: true
        transform:
          translate: [0, 1, 0]
          rotate_deg: [0, 0, 0]
          scale: [2, 0.1, 1]
        material:
          color: [1, 1, 1]
          ambient: 0.1
          diffuse: 0.9
          specular: 0.9
          shininess: 200
          reflective: 0
          transparency: 0
          refractive_index: 1)yaml");

  const auto parsed = scene_io::ParseSceneYaml(group_yaml);

  REQUIRE(parsed.ok);
  REQUIRE(parsed.scene.objects.size() == 1);
  REQUIRE(parsed.scene.objects[0].IsGroup());
  REQUIRE(parsed.scene.objects[0].Group().children.size() == 1);
  CHECK(parsed.scene.objects[0].Group().children[0].Shape().type == scene::ObjectType::kCube);
  const auto serialized = scene_io::SerializeSceneYaml(parsed.scene);
  CHECK(serialized.find("type: group") != std::string::npos);
  const auto reparsed = scene_io::ParseSceneYaml(serialized);
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == parsed.scene);
}

SCENARIO("scene yaml: rejects invalid scenes with useful errors") {
  CHECK_FALSE(scene_io::ParseSceneYaml("camera: {}\nlights: []\nobjects: []\n").ok);
  CHECK_FALSE(scene_io::ParseSceneYaml(std::string{kMinimalSceneYaml} + "\nextra: 1\n").ok);

  auto unknown_object = std::string{kFullSceneYaml};
  unknown_object.replace(unknown_object.find("type: plane"), std::string{"type: plane"}.size(), "type: torusy");
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

SCENARIO("scene yaml: loads relative OBJ sources and preserves them through Save As") {
  const auto path = std::filesystem::path{RAY_TRACER_ASSET_DIR} / "scenes" / "obj_model.yaml";
  const auto loaded = scene_io::LoadSceneYaml(path);
  INFO(loaded.error);
  REQUIRE(loaded.ok);
  REQUIRE(loaded.scene.objects.size() == 1);
  const auto& shape = loaded.scene.objects[0].Shape();
  CHECK(shape.type == scene::ObjectType::kObj);
  CHECK(std::filesystem::path(shape.source).is_absolute());
  CHECK(std::filesystem::is_regular_file(shape.source));
  const auto serialized = scene_io::SerializeSceneYaml(loaded.scene);
  const auto reparsed = scene_io::ParseSceneYaml(serialized);
  REQUIRE(reparsed.ok);
  CHECK(reparsed.scene == loaded.scene);

  const auto saved_path = std::filesystem::temp_directory_path() / "ray_tracer_obj_save_as.yaml";
  REQUIRE(scene_io::SaveSceneYaml(loaded.scene, saved_path).ok);
  const auto reloaded = scene_io::LoadSceneYaml(saved_path);
  std::filesystem::remove(saved_path);
  REQUIRE(reloaded.ok);
  CHECK(reloaded.scene == loaded.scene);
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
