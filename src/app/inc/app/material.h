#pragma once
#include "color.h"
namespace ray_tracer {
class Material {
 public:
  constexpr Material() noexcept : color_(1, 1, 1, 1) {}
  ~Material() = default;
  auto operator==(const Material& rhs) const -> bool {
    return color_ == rhs.color_ && ambient_ == rhs.ambient_ && diffuse_ == rhs.diffuse_ && specular_ == rhs.specular_ &&
           shininess_ == rhs.shininess_;
  }
  [[nodiscard]] auto GetColor() const -> Color { return color_; }
  void SetColor(const Color& color) { color_ = color; }
  [[nodiscard]] auto GetAmbient() const -> double { return ambient_; }
  void SetAmbient(const double ambient) { ambient_ = ambient; }
  [[nodiscard]] auto GetDiffuse() const -> double { return diffuse_; }
  void SetDiffuse(const double diffuse) { diffuse_ = diffuse; }
  [[nodiscard]] auto GetSpecular() const -> double { return specular_; }
  void SetSpecular(const double specular) { specular_ = specular; }
  [[nodiscard]] auto GetShininess() const -> double { return shininess_; }
  void SetShininess(const double shininess) { shininess_ = shininess; }

 private:
  Color color_;
  double ambient_{0.1};
  double diffuse_{0.9};
  double specular_{0.9};
  double shininess_{200.0};
};
}  // namespace ray_tracer
