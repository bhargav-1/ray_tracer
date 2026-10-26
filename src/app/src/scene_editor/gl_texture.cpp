#include "app/scene_editor/gl_texture.h"

#include <vector>

#include <GL/gl.h>

namespace ray_tracer::scene_editor {

GlTexture::~GlTexture() { Reset(); }

auto GlTexture::UploadRgb(const int width, const int height, const std::vector<unsigned char>& rgb) -> void {
  if (texture_id_ == 0) {
    glGenTextures(1, &texture_id_);
  }
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
}

auto GlTexture::Reset() -> void {
  if (texture_id_ != 0) {
    glDeleteTextures(1, &texture_id_);
    texture_id_ = 0;
  }
}

}  // namespace ray_tracer::scene_editor
