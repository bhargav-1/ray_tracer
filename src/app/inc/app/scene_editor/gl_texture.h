#pragma once

#include <vector>

#include <GLFW/glfw3.h>

namespace ray_tracer::scene_editor {

class GlTexture {
 public:
  GlTexture() = default;
  GlTexture(const GlTexture&) = delete;
  auto operator=(const GlTexture&) -> GlTexture& = delete;
  GlTexture(GlTexture&&) = delete;
  auto operator=(GlTexture&&) -> GlTexture& = delete;
  ~GlTexture();

  [[nodiscard]] auto Id() const -> GLuint { return texture_id_; }
  [[nodiscard]] auto Valid() const -> bool { return texture_id_ != 0; }
  auto UploadRgb(int width, int height, const std::vector<unsigned char>& rgb) -> void;
  auto Reset() -> void;

 private:
  GLuint texture_id_{0};
};

}  // namespace ray_tracer::scene_editor
