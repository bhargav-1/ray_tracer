#include <chrono>
#include <thread>
#include <cstdio>
#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers
#include <app/rendering/intersection.h>
#include <app/rendering/ray_tracing_engine.h>
#include <app/shapes/sphere.h>
#include <fmt/base.h>
#include <app/materials/color.h>

#include "app/rendering/canvas.h"
#include "app/primitives/matrix.h"
#include "app/primitives/point.h"
#include "app/primitives/tuple.h"

static void GlfwErrorCallback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
auto main(int /*unused*/, char* * /*unused*/) -> int {  // NOLINT
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    return 1;
  }
  const auto* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(1200, 1200, "Dear ImGui Clock Face", nullptr, nullptr);
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

    static GLuint texture_id = 0;
    static bool rendered = false;
    if (!rendered) {
      auto ray_origin = ray_tracer::Point(0, 0, -5);
      auto wall_z = 1000.0;
      constexpr auto kWallSize = 700.0;
      constexpr auto kCanvasPixels = 1000;
      constexpr auto kPixelSize = kWallSize / kCanvasPixels;
      constexpr auto kHalf = kWallSize / 2;

      ray_tracer::Sphere shape(1);
      auto transform = ray_tracer::Matrix<4, 4>::Scaling(1, 1, 1);
      transform =
          transform * ray_tracer::Matrix<4, 4>::Shearing(
                          {.proportion_to_y = 1, .proportion_to_z = 0}, {.proportion_to_x = 0, .proportion_to_z = 0}, {});
      shape.SetTransform(transform);

      constexpr ray_tracer::Color kRed{255, 0, 0, 255};
      std::vector<unsigned char> rgb(kCanvasPixels * kCanvasPixels * 3, 0);
      for (auto y = 0; y < kCanvasPixels; ++y) {
        auto world_y = kHalf - kPixelSize * y;
        for (auto x = 0; x < kCanvasPixels; ++x) {
          auto world_x = -kHalf + kPixelSize * x;
          ray_tracer::Point const position{world_x, world_y, wall_z};
          ray_tracer::Ray ray{ray_origin, ray_tracer::Vec3(position - ray_origin).Normalize()};
          auto xs = shape.Intersect(ray);
          if (Hit(xs).has_value()) {
            const auto rgb_offset = (static_cast<size_t>(y) * kCanvasPixels + x) * 3;
            rgb[rgb_offset + 0] = static_cast<unsigned char>(kRed.GetRed());
            rgb[rgb_offset + 1] = static_cast<unsigned char>(kRed.GetGreen());
            rgb[rgb_offset + 2] = static_cast<unsigned char>(kRed.GetBlue());
          }
        }
      }
      glGenTextures(1, &texture_id);
      glBindTexture(GL_TEXTURE_2D, texture_id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kCanvasPixels, kCanvasPixels, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
      rendered = true;
      std::cout << " Rendering Complete \n";
    }
    {
      constexpr int kCanvasPixels = 1000;
      const ImVec2 avail = ImGui::GetContentRegionAvail();
      const float img_scale = std::min(avail.x / kCanvasPixels, avail.y / kCanvasPixels);
      // NOLINTNEXTLINE(performance-no-int-to-ptr): ImGui stores an opaque OpenGL handle.
      ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id)),
                   ImVec2(kCanvasPixels * img_scale, kCanvasPixels * img_scale));
    }
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
