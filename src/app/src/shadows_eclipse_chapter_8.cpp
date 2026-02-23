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
#include "app/transformations.h"
#include "app/world.h"
#include "app/computations.h"
#include "app/ray_tracing_engine.h"

static void GlfwErrorCallback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void RenderScene(ImDrawList* canvas) {
  static std::optional<std::vector<ray_tracer::Color>> cached_pixels;
  static int cached_width = 0;
  static int cached_height = 0;
  if (!cached_pixels.has_value()) {
    const auto cache_start = std::chrono::high_resolution_clock::now();
    ray_tracer::World world{};
    auto sphere1 = ray_tracer::Sphere(1);
    sphere1.SetTransform(ray_tracer::Matrix<4, 4>::Translation(0, 0, -100) *
                         ray_tracer::Matrix<4, 4>::Scaling(20, 20, 20));
    // Sun: brighter, shinier so the rim reads as an eclipse.
    sphere1.SetMaterial(ray_tracer::Material{ray_tracer::Color{1, 1, 1.0}, 0.35, 0.9, 0.5, 200});
    auto sphere2 = ray_tracer::Sphere(2);
    sphere2.SetTransform(ray_tracer::Matrix<4, 4>::Translation(0, 0, -40) *
                        ray_tracer::Matrix<4, 4>::Scaling(9.8, 9.8, 9.8));
    // Moon: dark and matte.
    sphere2.SetMaterial(ray_tracer::Material{ray_tracer::Color{0.1, 0.1, 0.1}, 0.05, 0.7, 0.0, 10});
    world.AddObject(std::make_shared<ray_tracer::Sphere>(sphere1));
    world.AddObject(std::make_shared<ray_tracer::Sphere>(sphere2));

    world.AddLight(
        ray_tracer::PointLight(ray_tracer::Point(0, 0, 10), ray_tracer::Color(1.3, 1.3, 1.3)));
    world.AddLight(
        ray_tracer::PointLight(ray_tracer::Point(10, 10, 10), ray_tracer::Color(1, 1, 1)));

    auto camera = ray_tracer::Camera(1000, 500, std::numbers::pi / 3);
    camera.SetTransform(ray_tracer::ViewTransform(ray_tracer::Point(0, 0, 20),
                                                  ray_tracer::Point(0, 0, -10),
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

  const auto& pixels = *cached_pixels;
  const ImVec2 start = ImGui::GetCursorScreenPos();
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
  ImVec2 offset = start;
  if (avail.x > width * scale) {
    offset.x += (avail.x - width * scale) * 0.5F;
  }
  if (avail.y > height * scale) {
    offset.y += (avail.y - height * scale) * 0.5F;
  }

  for (int y = 0; y < cached_height; ++y) {
    for (int x = 0; x < cached_width; ++x) {
      const auto& color = pixels[static_cast<size_t>(y) * static_cast<size_t>(cached_width) +
                                 static_cast<size_t>(x)];
      const auto red = static_cast<int>(std::clamp(color.GetRed(), 0.0, 1.0) * 255.0);
      const auto green = static_cast<int>(std::clamp(color.GetGreen(), 0.0, 1.0) * 255.0);
      const auto blue = static_cast<int>(std::clamp(color.GetBlue(), 0.0, 1.0) * 255.0);
      const ImU32 packed = IM_COL32(red, green, blue, 255);
      const ImVec2 min_point{offset.x + static_cast<float>(x) * scale,
                             offset.y + static_cast<float>(y) * scale};
      const ImVec2 max_point{min_point.x + scale, min_point.y + scale};
      canvas->AddRectFilled(min_point, max_point, packed);
    }
  }

  ImGui::Dummy(ImVec2(width * scale, height * scale));
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

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(1200, 1200, "ECLIPSE!", nullptr, nullptr);
  if (window == nullptr) {
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  constexpr auto kClearColor = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  // Main loop
  io.IniFilename = nullptr;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
      continue;
    }
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(1000, 900), ImGuiCond_Once);
    ImGui::Begin("Ray Casting:");

    ImDrawList* canvas = ImGui::GetWindowDrawList();

    RenderScene(canvas);
    ImGui::End();

    // Rendering
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

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
