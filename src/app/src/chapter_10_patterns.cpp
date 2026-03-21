// Chapter 10 — Patterns showcase
//
// Scene layout:
//   Camera at (1, 3, -7) looking at (1, 0.5, 0), FOV 60°
//
//   Floor (Plane):
//     CheckersPattern of two RadialGradientPatterns
//       Tile A: cream → brown concentric rings (scale 0.15)
//       Tile B: light blue → dark blue concentric rings (scale 0.15)
//
//   Front row (left to right, z=0):
//     Sphere 1  r=0.4  x=-3.2  StripePattern (red/white, rotated 45°)
//     Sphere 2  r=0.6  x=-1.8  GradientPattern (green → purple)
//     Sphere 3  r=0.8  x=-0.2  RingPattern (blue/yellow, tilted 60°)
//     Sphere 4  r=1.0  x= 1.5  CheckersPattern (red/cream, scale 0.3)
//     Sphere 5  r=0.7  x= 3.2  RadialGradientPattern (gold → purple, scale 0.25)
//     Sphere 6  r=0.5  x= 4.6  BlendedPattern (red/yellow stripes + blue/green stripes = plaid)
//
//   Back row (z=4):
//     Sphere 7  r=1.5  x= 1.0  Nested CheckersPattern of two StripePatterns
//                                 Tile A: green stripes (scale 0.15)
//                                 Tile B: orange/purple stripes rotated 90° (scale 0.15)
//
//   Light: point light at (-10, 10, -10), white

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>
#include <memory>
#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers
#include <chrono>
#include <numbers>
#include "app/camera.h"
#include "app/color.h"
#include "app/intersection.h"
#include "app/matrix.h"
#include "app/sphere.h"
#include "app/plane.h"
#include "app/pattern.h"
#include "app/transformations.h"
#include "app/ray_tracing_engine.h"
#include "app/world.h"

static void GlfwErrorCallback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void RenderScene() {
  static std::optional<std::vector<ray_tracer::Color>> cached_pixels;
  static int cached_width = 0;
  static int cached_height = 0;
  if (!cached_pixels.has_value()) {
    const auto cache_start = std::chrono::high_resolution_clock::now();
    ray_tracer::World world{};

    // Floor plane — checkers of two radial gradients
    auto floor = ray_tracer::Plane(1);
    auto floor_a = std::make_shared<ray_tracer::RadialGradientPattern>(
        ray_tracer::Color{0.9, 0.9, 0.85}, ray_tracer::Color{0.4, 0.2, 0.1});
    floor_a->SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.15, 0.15, 0.15));
    auto floor_b = std::make_shared<ray_tracer::RadialGradientPattern>(
        ray_tracer::Color{0.6, 0.8, 0.9}, ray_tracer::Color{0.1, 0.2, 0.4});
    floor_b->SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.15, 0.15, 0.15));
    auto floor_pat = ray_tracer::CheckersPattern(floor_a, floor_b);
    auto floor_mat = ray_tracer::Material{ray_tracer::Color{1, 0.9, 0.9}, 0.1, 0.9, 0.0, 200};
    floor_mat.SetPattern(std::make_shared<ray_tracer::CheckersPattern>(floor_pat));
    floor.SetMaterial(floor_mat);
    world.AddObject(std::make_shared<ray_tracer::Plane>(floor));

    // Sphere 1 — stripe
    auto s1 = ray_tracer::Sphere(2);
    s1.SetTransform(ray_tracer::Matrix<4, 4>::Translation(-3.2, 0.4, 0) *
                    ray_tracer::Matrix<4, 4>::Scaling(0.4, 0.4, 0.4));
    auto s1_pat = ray_tracer::StripePattern(
        ray_tracer::Color{1, 0.2, 0.2}, ray_tracer::Color{1, 1, 1});
    s1_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.2, 0.2, 0.2) *
                               ray_tracer::Matrix<4, 4>::RotationZ(std::numbers::pi / 4));
    auto s1_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s1_mat.SetPattern(std::make_shared<ray_tracer::StripePattern>(s1_pat));
    s1.SetMaterial(s1_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s1));

    // Sphere 2 — gradient
    auto s2 = ray_tracer::Sphere(3);
    s2.SetTransform(ray_tracer::Matrix<4, 4>::Translation(-1.8, 0.6, 0) *
                    ray_tracer::Matrix<4, 4>::Scaling(0.6, 0.6, 0.6));
    auto s2_pat = ray_tracer::GradientPattern(
        ray_tracer::Color{0.2, 0.8, 0.3}, ray_tracer::Color{0.8, 0.2, 0.8});
    auto s2_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s2_mat.SetPattern(std::make_shared<ray_tracer::GradientPattern>(s2_pat));
    s2.SetMaterial(s2_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s2));

    // Sphere 3 — ring
    auto s3 = ray_tracer::Sphere(4);
    s3.SetTransform(ray_tracer::Matrix<4, 4>::Translation(-0.2, 0.8, 0) *
                    ray_tracer::Matrix<4, 4>::Scaling(0.8, 0.8, 0.8));
    auto s3_pat = ray_tracer::RingPattern(
        ray_tracer::Color{0.1, 0.3, 0.8}, ray_tracer::Color{0.9, 0.9, 0.2});
    s3_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.12, 0.12, 0.12) *
                               ray_tracer::Matrix<4, 4>::RotationX(std::numbers::pi / 3));
    auto s3_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s3_mat.SetPattern(std::make_shared<ray_tracer::RingPattern>(s3_pat));
    s3.SetMaterial(s3_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s3));

    // Sphere 4 — checkers (largest)
    auto s4 = ray_tracer::Sphere(5);
    s4.SetTransform(ray_tracer::Matrix<4, 4>::Translation(1.5, 1.0, 0) *
                    ray_tracer::Matrix<4, 4>::Scaling(1.0, 1.0, 1.0));
    auto s4_pat = ray_tracer::CheckersPattern(
        ray_tracer::Color{0.8, 0.2, 0.1}, ray_tracer::Color{0.0, 0.9, 1});
    s4_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.3, 0.3, 0.3));
    auto s4_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s4_mat.SetPattern(std::make_shared<ray_tracer::CheckersPattern>(s4_pat));
    s4.SetMaterial(s4_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s4));

    // Sphere 5 — radial gradient
    auto s5 = ray_tracer::Sphere(6);
    s5.SetTransform(ray_tracer::Matrix<4, 4>::Translation(3.2, 0.7, 0) *
                    ray_tracer::Matrix<4, 4>::Scaling(0.7, 0.7, 0.7));
    auto s5_pat = ray_tracer::RadialGradientPattern(
        ray_tracer::Color{1.0, 0.8, 0.0}, ray_tracer::Color{0.2, 0.0, 0.6});
    s5_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.25, 0.25, 0.25));
    auto s5_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s5_mat.SetPattern(std::make_shared<ray_tracer::RadialGradientPattern>(s5_pat));
    s5.SetMaterial(s5_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s5));

    // Sphere 6 — blended (plaid)
    auto s6 = ray_tracer::Sphere(7);
    s6.SetTransform(ray_tracer::Matrix<4, 4>::Translation(4.6, 0.5, 0) *
                    ray_tracer::Matrix<4, 4>::Scaling(0.5, 0.5, 0.5));
    auto bl_a = std::make_shared<ray_tracer::StripePattern>(
        ray_tracer::Color{0.9, 0.1, 0.1}, ray_tracer::Color{0.9, 0.9, 0.2});
    bl_a->SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.05, 0.05, 0.05));
    auto bl_b = std::make_shared<ray_tracer::StripePattern>(
        ray_tracer::Color{0.1, 0.1, 0.9}, ray_tracer::Color{0.2, 0.9, 0.2});
    bl_b->SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.05, 0.05, 0.05) *
                              ray_tracer::Matrix<4, 4>::RotationZ(std::numbers::pi / 2));
    auto s6_pat = ray_tracer::BlendedPattern(bl_a, bl_b);
    auto s6_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s6_mat.SetPattern(std::make_shared<ray_tracer::BlendedPattern>(s6_pat));
    s6.SetMaterial(s6_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s6));

    // Sphere 7 — nested (checkers of stripes), behind the row
    auto s7 = ray_tracer::Sphere(8);
    s7.SetTransform(ray_tracer::Matrix<4, 4>::Translation(1.0, 1.5, 4) *
                    ray_tracer::Matrix<4, 4>::Scaling(1.5, 1.5, 1.5));
    auto n_a = std::make_shared<ray_tracer::StripePattern>(
        ray_tracer::Color{0.1, 0.8, 0.1}, ray_tracer::Color{0.0, 0.3, 0.0});
    n_a->SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.15, 0.15, 0.15));
    auto n_b = std::make_shared<ray_tracer::StripePattern>(
        ray_tracer::Color{0.9, 0.5, 0.0}, ray_tracer::Color{0.5, 0.0, 0.9});
    n_b->SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.15, 0.15, 0.15) *
                             ray_tracer::Matrix<4, 4>::RotationY(std::numbers::pi / 2));
    auto s7_pat = ray_tracer::CheckersPattern(n_a, n_b);
    s7_pat.SetPatternTransform(ray_tracer::Matrix<4, 4>::Scaling(0.5, 0.5, 0.5));
    auto s7_mat = ray_tracer::Material{ray_tracer::Color{1, 1, 1}, 0.1, 0.7, 0.3, 200};
    s7_mat.SetPattern(std::make_shared<ray_tracer::CheckersPattern>(s7_pat));
    s7.SetMaterial(s7_mat);
    world.AddObject(std::make_shared<ray_tracer::Sphere>(s7));

    world.AddLight(ray_tracer::PointLight(ray_tracer::Point(-10, 10, -10), ray_tracer::Color(1, 1, 1)));
    auto camera = ray_tracer::Camera(1000, 1000, std::numbers::pi / 3);
    camera.SetTransform(ray_tracer::ViewTransform(ray_tracer::Point(1.0, 3.5, -9),
                                                  ray_tracer::Point(1.0, 0.5, 0),
                                                  ray_tracer::Vec3(0, 1, 0)));

    cached_width = camera.GetHorizontalSize();
    cached_height = camera.GetVerticalSize();
    std::vector<ray_tracer::Color> pixels(static_cast<size_t>(cached_width) *
                                          static_cast<size_t>(cached_height));
    const unsigned int worker_count = std::max(1U, std::thread::hardware_concurrency());
    const int rows_per_worker = (cached_height + static_cast<int>(worker_count) - 1) /
                                static_cast<int>(worker_count);
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

  GLFWwindow* window = glfwCreateWindow(1200, 1200, "10: Patterns", nullptr, nullptr);
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
    ImGui::SetNextWindowSize(ImVec2(1000, 1000), ImGuiCond_Once);
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
