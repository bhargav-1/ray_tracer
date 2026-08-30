#pragma once

#include <optional>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace ray_tracer::scene {

struct Vec3 {
  float x{0.0F};
  float y{0.0F};
  float z{0.0F};

  constexpr auto operator==(const Vec3&) const -> bool = default;
};

struct ColorRgb {
  float r{1.0F};
  float g{1.0F};
  float b{1.0F};

  constexpr auto operator==(const ColorRgb&) const -> bool = default;
};

struct Resolution {
  int width{800};
  int height{400};

  constexpr auto operator==(const Resolution&) const -> bool = default;
};

struct Transform {
  Vec3 translate{0.0F, 0.0F, 0.0F};
  Vec3 rotate_deg{0.0F, 0.0F, 0.0F};
  Vec3 scale{1.0F, 1.0F, 1.0F};

  constexpr auto operator==(const Transform&) const -> bool = default;
};

struct Camera {
  Vec3 from{0.0F, 1.5F, -5.0F};
  Vec3 to{0.0F, 1.0F, 0.0F};
  Vec3 up{0.0F, 1.0F, 0.0F};
  float fov_deg{60.0F};
  Resolution resolution{800, 400};

  constexpr auto operator==(const Camera&) const -> bool = default;
};

struct Light {
  Vec3 position{-10.0F, 10.0F, -10.0F};
  ColorRgb color{1.0F, 1.0F, 1.0F};
  bool visible{true};

  constexpr auto operator==(const Light&) const -> bool = default;
};

struct AxialBounds {
  float minimum{-1.0F};
  float maximum{1.0F};
  bool closed{true};

  constexpr auto operator==(const AxialBounds&) const -> bool = default;
};

enum class ObjectType : std::uint8_t { kSphere, kPlane, kCube, kCylinder, kCone };

enum class PatternType : std::uint8_t {
  kSolid,
  kStripe,
  kGradient,
  kRing,
  kCheckers,
  kRadialGradient,
  kUvCheckers,
  kBlended,
  kPerturbed
};

struct Pattern {  // NOLINT(misc-no-recursion)
  PatternType type{PatternType::kSolid};
  ColorRgb color{1.0F, 1.0F, 1.0F};
  Transform transform;
  std::vector<Pattern> children;
  float perturb_scale{0.2F};
  int width{2};
  int height{2};

  // NOLINTNEXTLINE(misc-no-recursion)
  auto operator==(const Pattern&) const -> bool = default;
};

struct Material {
  ColorRgb color{1.0F, 1.0F, 1.0F};
  float ambient{0.1F};
  float diffuse{0.9F};
  float specular{0.9F};
  float shininess{200.0F};
  float reflective{0.0F};
  float transparency{0.0F};
  float refractive_index{1.0F};
  std::optional<Pattern> pattern;

  auto operator==(const Material&) const -> bool = default;
};

struct ShapeSpec {
  ObjectType type{ObjectType::kSphere};
  AxialBounds axial_bounds;
  Material material;
  bool cast_shadow{true};

  auto operator==(const ShapeSpec&) const -> bool = default;
};

struct Object;

struct GroupSpec {  // NOLINT(misc-no-recursion)
  std::vector<Object> children;

  // NOLINTNEXTLINE(misc-no-recursion)
  auto operator==(const GroupSpec&) const -> bool = default;
};

struct Object {  // NOLINT(misc-no-recursion)
  std::string name;
  Transform transform;
  bool visible{true};
  std::variant<ShapeSpec, GroupSpec> content;

  [[nodiscard]] auto IsGroup() const -> bool { return std::holds_alternative<GroupSpec>(content); }
  [[nodiscard]] auto Shape() -> ShapeSpec& { return std::get<ShapeSpec>(content); }
  [[nodiscard]] auto Shape() const -> const ShapeSpec& { return std::get<ShapeSpec>(content); }
  [[nodiscard]] auto Group() -> GroupSpec& { return std::get<GroupSpec>(content); }
  [[nodiscard]] auto Group() const -> const GroupSpec& { return std::get<GroupSpec>(content); }

  // NOLINTNEXTLINE(misc-no-recursion)
  auto operator==(const Object&) const -> bool = default;
};

struct Scene {
  int version{1};
  std::optional<Camera> camera;
  std::vector<Light> lights;
  std::vector<Object> objects;

  auto operator==(const Scene&) const -> bool = default;
};

}  // namespace ray_tracer::scene
