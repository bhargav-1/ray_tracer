#pragma once
#include "color.h"
namespace ray_tracer {
class Material {
 public:
  constexpr Material() noexcept : color_(1, 1, 1, 1) {}
  ~Material() = default;
  constexpr auto operator==(const Material& rhs) const -> bool {
    return color_ == rhs.color_ && ambient_ == rhs.ambient_ && diffuse_ == rhs.diffuse_ && specular_ == rhs.specular_ &&
           shininess_ == rhs.shininess_;
  }
  [[nodiscard]] constexpr auto GetColor() const -> Color { return color_; }
  constexpr void SetColor(const Color& color) { color_ = color; }
  [[nodiscard]] constexpr auto GetAmbient() const -> double { return ambient_; }
  constexpr void SetAmbient(const double ambient) { ambient_ = ambient; }
  [[nodiscard]] constexpr auto GetDiffuse() const -> double { return diffuse_; }
  constexpr void SetDiffuse(const double diffuse) { diffuse_ = diffuse; }
  [[nodiscard]] constexpr auto GetSpecular() const -> double { return specular_; }
  constexpr void SetSpecular(const double specular) { specular_ = specular; }
  [[nodiscard]] constexpr auto GetShininess() const -> double { return shininess_; }
  constexpr void SetShininess(const double shininess) { shininess_ = shininess; }

 private:
  Color color_;
  double ambient_{0.1};
  double diffuse_{0.9};
  double specular_{0.9};
  double shininess_{200.0};
};
}  // namespace ray_tracer
