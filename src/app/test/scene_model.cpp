#include <app/scene/scene.h>
#include <catch2/catch_test_macros.hpp>

namespace ray_tracer {
namespace {

auto Solid(scene::ColorRgb color) -> scene::Pattern {
  scene::Pattern pattern;
  pattern.type = scene::PatternType::kSolid;
  pattern.color = color;
  return pattern;
}

}  // namespace

SCENARIO("scene model: empty scenes can be constructed") {
  const scene::Scene scene;
  CHECK(scene.version == 1);
  CHECK_FALSE(scene.camera.has_value());
  CHECK(scene.lights.empty());
  CHECK(scene.objects.empty());
}

SCENARIO("scene model: scenes can contain camera, lights, spheres, planes, and cubes") {
  scene::Scene scene;
  scene.camera = scene::Camera{};
  scene.lights.push_back(scene::Light{});
  scene::Object sphere;
  sphere.name = "sphere";
  sphere.type = scene::ObjectType::kSphere;
  scene.objects.push_back(sphere);
  scene::Object plane;
  plane.name = "plane";
  plane.type = scene::ObjectType::kPlane;
  scene.objects.push_back(plane);
  scene::Object cube;
  cube.name = "cube";
  cube.type = scene::ObjectType::kCube;
  scene.objects.push_back(cube);

  CHECK(scene.camera.has_value());
  CHECK(scene.lights.size() == 1);
  CHECK(scene.lights[0].visible);
  CHECK(scene.objects.size() == 3);
  CHECK(scene.objects[0].type == scene::ObjectType::kSphere);
  CHECK(scene.objects[1].type == scene::ObjectType::kPlane);
  CHECK(scene.objects[2].type == scene::ObjectType::kCube);
}

SCENARIO("scene model: recursive patterns can be constructed") {
  const auto red = Solid({1.0F, 0.0F, 0.0F});
  const auto blue = Solid({0.0F, 0.0F, 1.0F});

  scene::Pattern stripe;
  stripe.type = scene::PatternType::kStripe;
  stripe.children = {red, blue};

  scene::Pattern perturbed;
  perturbed.type = scene::PatternType::kPerturbed;
  perturbed.perturb_scale = 0.6F;
  perturbed.children = {stripe};

  CHECK(red.type == scene::PatternType::kSolid);
  CHECK(stripe.children.size() == 2);
  CHECK(perturbed.children.size() == 1);
  CHECK(perturbed.children[0].type == scene::PatternType::kStripe);
}

SCENARIO("scene model: transform defaults are identity authoring values") {
  const scene::Transform transform;
  CHECK(transform.translate == scene::Vec3{0.0F, 0.0F, 0.0F});
  CHECK(transform.rotate_deg == scene::Vec3{0.0F, 0.0F, 0.0F});
  CHECK(transform.scale == scene::Vec3{1.0F, 1.0F, 1.0F});
}

}  // namespace ray_tracer
