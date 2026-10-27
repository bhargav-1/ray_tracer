#pragma once

#include <filesystem>
#include <system_error>

namespace ray_tracer {

// Prefer assets beside an installed executable, then the development checkout.
inline auto AssetDirectory(const std::filesystem::path& executable = "/proc/self/exe") -> std::filesystem::path {
  std::error_code error;
  const auto resolved = std::filesystem::canonical(executable, error);
  if (!error) {
    const auto installed = (resolved.parent_path().parent_path() / RAY_TRACER_INSTALL_ASSET_DIR).lexically_normal();
    if (std::filesystem::is_directory(installed, error)) {
      return installed;
    }
  }
  return RAY_TRACER_ASSET_DIR;
}

}  // namespace ray_tracer
