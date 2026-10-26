// Chapter 10 — Perturbed Patterns showcase
//
// Reproducing the book's page 140 image:
//   Floor: perturbed red/white checkers
//   Center sphere (large): perturbed two-tone green checkers
//   Left sphere (small): perturbed green ring pattern
//   Right sphere: gradient (orange → yellow)

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include <vector>
#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers
#include <chrono>
#include <numbers>
#include "app/materials/color.h"
#include "app/materials/pattern.h"
#include "app/primitives/matrix.h"
#include "app/primitives/transformations.h"
#include "app/rendering/camera.h"
#include "app/rendering/intersection.h"
#include "app/rendering/ray_tracing_engine.h"
#include "app/rendering/world.h"
#include "app/shapes/plane.h"
#include "app/shapes/sphere.h"

static void GlfwErrorCallback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

template <typename T>
auto Perturbed(T pattern, double scale = 0.8) -> std::shared_ptr<ray_tracer::PerturbedPattern> {
  return std::make_shared<ray_tracer::PerturbedPattern>(std::make_shared<T>(std::move(pattern)), scale);
}

static void RenderScene() {
  static std::optional<std::vector<ray_tracer::Color>> cached_pixels;
  static int cached_width = 0;
  static int cached_height = 0;
  if (!cached_pixels.has_value()) {
    const auto cache_start = std::chrono::high_resolution_clock::now();
    ray_tracer::World world{};

    // Floor — perturbed checkers
    auto floor = ray_tracer::Plane(1);
    auto floor_pat =
        ray_tracer::CheckersPattern(ray_tracer::Color{0.85, 0.15, 0.15}, ray_tracer::Color{0.95, 0.95, 0.95});
    floor_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2));
    auto floor_mat = ray_tracer::Material{ray_tracer::Color{1, 0.9, 0.9}, 0.1, 0.9, 0.0, 200};
    floor_mat.SetPattern(Perturbed(floor_pat, 0.6));
    floor.SetMaterial(floor_mat);
    world.AddObject(std::make_shared<ray_tracer::Plane>(floor));

    // Center sphere — perturbed stripes (two-tone green)
    auto center = ray_tracer::Sphere(2);
    center.SetTransform(ray_tracer::Matrix<4, 4>::Translation(0, 1, 0));
    auto center_pat = ray_tracer::StripePattern(ray_tracer::Color{0.05, 0.5, 0.1}, ray_tracer::Color{0.15, 0.7, 0.2});
    center_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.25, 0.25, 0.25));
    auto center_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    center_mat.SetPattern(Perturbed(center_pat, 1.0));
    center.SetMaterial(center_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(center));

    // Left sphere — perturbed stripes (two-tone green)
    auto left = ray_tracer::Sphere(3);
    left.SetTransform(ray_tracer::Matrix<4, 4>::Translation(-1.5, 0.33, -0.75) *
                      ray_tracer::Matrix<4, 4>::Scaling(0.33, 0.33, 0.33));
    auto left_pat = ray_tracer::StripePattern(ray_tracer::Color{0.05, 0.5, 0.1}, ray_tracer::Color{0.2, 0.65, 0.25});
    left_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.2, 0.2, 0.2));
    auto left_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    left_mat.SetPattern(Perturbed(left_pat, 0.8));
    left.SetMaterial(left_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(left));

    // Right sphere — solid orange
    auto right = ray_tracer::Sphere(4);
    right.SetTransform(ray_tracer::Matrix<4, 4>::Translation(1.5, 0.5, -0.5) *
                       ray_tracer::Matrix<4, 4>::Scaling(0.5, 0.5, 0.5));
    auto right_mat = ray_tracer::Material{ray_tracer::Color{1.0, 0.55, 0.0}, 0.1, 0.7, 0.3, 200};
    right.SetMaterial(right_mat);
    right.SetMaterial(right_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(right));

    world.AddLight(ray_tracer::PointLight(ray_tracer::Point(-10, 10, -10), ray_tracer::Color(1, 1, 1)));
    auto camera = ray_tracer::Camera(1200, 600, std::numbers::pi / 3);
    camera.SetTransform(ray_tracer::ViewTransform(
        ray_tracer::Point(0, 1.5, -5), ray_tracer::Point(0, 1, 0), ray_tracer::Vec3(0, 1, 0)));

    cached_width = camera.GetHorizontalSize();
    cached_height = camera.GetVerticalSize();
    std::vector<ray_tracer::Color> pixels(static_cast<size_t>(cached_width) * static_cast<size_t>(cached_height));
    const unsigned int worker_count = std::max(1U, std::thread::hardware_concurrency());
    const int rows_per_worker = (cached_height + static_cast<int>(worker_count) - 1) / static_cast<int>(worker_count);
    std::atomic<int> rows_done{0};
    std::atomic<int> last_bucket{-1};
    std::vector<std::thread> workers;
    workers.reserve(worker_count);
    for (unsigned int worker = 0; worker < worker_count; ++worker) {
      const int start_row = static_cast<int>(worker) * rows_per_worker;
      if (start_row >= cached_height) {
        break;
      }
      const int end_row = std::min(cached_height, start_row + rows_per_worker);
      workers.emplace_back([&, start_row, end_row]() {
        for (int y = start_row; y < end_row; ++y) {
          for (int x = 0; x < cached_width; ++x) {
            const auto ray = camera.GetRayForPixel(x, y);
            const auto color = ray_tracer::ColorAt(world, ray);
            pixels[static_cast<size_t>(y) * static_cast<size_t>(cached_width) + static_cast<size_t>(x)] = color;
          }
          const int done = rows_done.fetch_add(1) + 1;
          const int percent = (done * 100) / cached_height;
          const int bucket = percent / 10;
          int last = last_bucket.load();
          while (bucket > last && !last_bucket.compare_exchange_weak(last, bucket)) {
          }
          if (bucket > last) {
            std::cout << "Render progress: " << bucket * 10 << "%\n";
          }
        }
      });
    }
    for (auto& worker : workers) {
      worker.join();
    }
    cached_pixels = std::move(pixels);
    const auto cache_end = std::chrono::high_resolution_clock::now();
    std::cout << "Initial render time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(cache_end - cache_start).count() << "ms\n";
  }

  static GLuint texture_id = 0;
  static bool texture_created = false;
  if (!texture_created) {
    const auto& pixels = *cached_pixels;
    std::vector<unsigned char> rgb(static_cast<size_t>(cached_width) * cached_height * 3);
    for (int y = 0; y < cached_height; ++y) {
      for (int x = 0; x < cached_width; ++x) {
        const auto pixel_index = static_cast<size_t>(y) * cached_width + x;
        const auto& color = pixels[pixel_index];
        const auto rgb_offset = pixel_index * 3;
        rgb[rgb_offset + 0] = static_cast<unsigned char>(std::clamp(color.GetRed(), 0.0, 1.0) * 255.0);
        rgb[rgb_offset + 1] = static_cast<unsigned char>(std::clamp(color.GetGreen(), 0.0, 1.0) * 255.0);
        rgb[rgb_offset + 2] = static_cast<unsigned char>(std::clamp(color.GetBlue(), 0.0, 1.0) * 255.0);
      }
    }
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cached_width, cached_height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
    texture_created = true;
  }

  const ImVec2 avail = ImGui::GetContentRegionAvail();
  const float width = static_cast<float>(cached_width);
  const float height = static_cast<float>(cached_height);
  float scale = 1.0F;
  if (avail.x > 0.0F && avail.y > 0.0F) {
    scale = std::min(avail.x / width, avail.y / height);
  }
  if (scale <= 0.0F) {
    scale = 1.0F;
  }
  const ImVec2 image_size{width * scale, height * scale};
  if (avail.x > image_size.x) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail.x - image_size.x) * 0.5F);
  }
  if (avail.y > image_size.y) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (avail.y - image_size.y) * 0.5F);
  }
  // NOLINTNEXTLINE(performance-no-int-to-ptr): ImGui stores an opaque OpenGL handle.
  ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id)), image_size);
}

// Main code
auto main(int /*unused*/, char** /*unused*/) -> int {  // NOLINT
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    return 1;
  }
  const auto* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(1200, 800, "10: Perturbed Patterns", nullptr, nullptr);
  if (window == nullptr) {
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  constexpr auto kClearColor = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  io.IniFilename = nullptr;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
      continue;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_Once);
    ImGui::Begin("Ray Casting:");

    RenderScene();
    ImGui::End();

    ImGui::Render();
    int display_w;
    int display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(
        kClearColor.x * kClearColor.w, kClearColor.y * kClearColor.w, kClearColor.z * kClearColor.w, kClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
