#include <chrono>
#include <thread>
#include <cstdio>
#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers
#include <app/intersection.h>
#include <app/sphere.h>
#include <fmt/base.h>
#include <src/app/inc/app/color.h>

#include "app/canvas.h"
#include "app/matrix.h"
#include "app/point.h"
#include "app/tuple.h"

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

    ImDrawList* canvas = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 window_size = ImGui::GetWindowSize();
    auto ray_origin = ray_tracer::Point(0, 0, -5);
    auto wall_z = 1000.0;
    constexpr auto kWallSize = 700.0;
    constexpr auto kCanvasPixels = 1000;
    constexpr auto kPixelSize = kWallSize / kCanvasPixels;
    constexpr auto kHalf = kWallSize / 2;

    float const center_x = pos.x + (window_size.x / 2);
    float const center_y = pos.y + (window_size.y / 2);
    ray_tracer::Matrix<4, 4> const canvas_transform =
        ray_tracer::Matrix<4, 4>::IdentityMatrix().Translate(center_x, center_y, 0);

    constexpr ray_tracer::Color kRed{255, 0, 0, 255};
    ImColor color = ImColor(static_cast<int>(kRed.GetRed()),
                            static_cast<int>(kRed.GetGreen()),
                            static_cast<int>(kRed.GetBlue()),
                            static_cast<int>(kRed.GetW()));
    ray_tracer::Point origin{0, 0, 0};
    auto print_center = canvas_transform * origin;
    ImVec2 center_vec = {static_cast<float>(print_center.GetX()), static_cast<float>(print_center.GetY())};
    std::cout << print_center << "\n";
    canvas->AddCircleFilled(center_vec, 10, color);

    ray_tracer::Sphere shape;
    auto transform = ray_tracer::Matrix<4, 4>::Scaling(1, 1, 1);
    transform =
        transform * ray_tracer::Matrix<4, 4>::Shearing(
                        {.proportion_to_y = 1, .proportion_to_z = 0}, {.proportion_to_x = 0, .proportion_to_z = 0}, {});
    shape.SetTransform(transform);
    // shape.SetTransform(ray_tracer::Matrix<4, 4>::Shearing(1, 0, 0, 0, 0, 0) *
                       // ray_tracer::Matrix<4, 4>::Scaling(2, 2, 2));
    for (auto y = 0; y < kCanvasPixels; ++y) {
      auto world_y = kHalf - kPixelSize * y;
      for (auto x = 0; x < kCanvasPixels; ++x) {
        auto world_x = -kHalf + kPixelSize * x;
        ray_tracer::Point const position{world_x, world_y, wall_z};
        ray_tracer::Ray ray{ray_origin, ray_tracer::Vec3(position - ray_origin).Normalize()};
        auto xs = Intersect(ray, shape);
        if (Hit(xs).has_value()) {
          canvas->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), color);
          // std::cout << "x : " << x << " y : " << y << '\n';
        }
      }
    }
    std::cout << " Rendering Complete \n";
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
