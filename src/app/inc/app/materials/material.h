#pragma once
#include <utility>
#include "app/materials/color.h"
#include "app/materials/pattern.h"
#include <memory>

namespace ray_tracer {
class Material {
 public:
  constexpr Material() noexcept : color_(1, 1, 1, 0) {}
  constexpr explicit Material(Color  color, const double ambient =0.1, const double diffuse =0.9, const double specular =0.9, const double shininess =200, const double reflective = 0.0, const double transparency = 0.0, const double refractive_index = 1.0) noexcept // NOLINT(bugprone-easily-swappable-parameters)
      : color_(std::move(color)), ambient_(ambient), diffuse_(diffuse), specular_(specular), shininess_(shininess) , reflective_(reflective), transparency_(transparency), refractive_index_(refractive_index){}
  ~Material() = default;
  constexpr auto operator==(const Material& rhs) const -> bool {
    return color_ == rhs.color_ && ambient_ == rhs.ambient_ && diffuse_ == rhs.diffuse_ && specular_ == rhs.specular_ &&
           shininess_ == rhs.shininess_ && reflective_ == rhs.reflective_ && transparency_ == rhs.transparency_ && refractive_index_ == rhs.refractive_index_;
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
  [[nodiscard]] auto HasPattern() const -> bool { return pattern_ != nullptr; }
  auto SetPattern(std::shared_ptr<Pattern> pattern) -> void {
    pattern_ = std::move(pattern);
  }
  [[nodiscard]] auto GetPattern() const -> const Pattern* { return pattern_.get(); }
  [[nodiscard]] auto GetReflective() const -> double { return reflective_; }
  constexpr void SetReflective(const double reflective) { reflective_ = reflective; }
  [[nodiscard]] constexpr auto GetTransparency() const -> double { return transparency_; }
  constexpr void SetTransparency(const double transparency) { transparency_ = transparency; }
  [[nodiscard]] constexpr auto GetRefractiveIndex() const -> double { return refractive_index_; }
  constexpr void SetRefractiveIndex(const double refractive_index) { refractive_index_ = refractive_index; }

 private:
  Color color_;
  double ambient_{0.1};
  double diffuse_{0.9};
  double specular_{0.9};
  double shininess_{200.0};
  double reflective_{0.0};
  std::shared_ptr<Pattern> pattern_;
  double transparency_{0.0};
  double refractive_index_{1.0};
};
}  // namespace ray_tracer
