#include <chrono>
#include <thread>
#include <cstdio>
#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers
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
    ImGui::Begin("Clock Face:");  // Create a window called "Hello, world!" and append into it.
    ImDrawList* canvas = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 window_size = ImGui::GetWindowSize();

    float const center_x = pos.x + (window_size.x / 2);
    float const center_y = pos.y + (window_size.y / 2);
    float const radius = std::min(window_size.x, window_size.y) * 0.4F;

    ray_tracer::Matrix<4, 4> canvas_transform =
        ray_tracer::Matrix<4, 4>::IdentityMatrix().Translate(center_x, center_y, 0);

    constexpr ray_tracer::Color kWhite{255, 255, 255, 255};
    ImColor color = ImColor(static_cast<int>(kWhite.GetRed()),
                            static_cast<int>(kWhite.GetGreen()),
                            static_cast<int>(kWhite.GetBlue()),
                            static_cast<int>(kWhite.GetW()));
    ray_tracer::Point origin{0, 0, 0};
    auto print_center = canvas_transform * origin;
    ImVec2 center_vec = {static_cast<float>(print_center.GetX()), static_cast<float>(print_center.GetY())};
    canvas->AddCircleFilled(center_vec, 10, color);
    auto transform = ray_tracer::Matrix<4, 4>::IdentityMatrix().Translate(0, -radius, 0);
    ray_tracer::Point mark_12 = transform * origin;
    ray_tracer::Point current_mark = mark_12;

    auto print_mark12 = canvas_transform * mark_12;
    canvas->AddCircleFilled(
        ImVec2(static_cast<float>(print_mark12.GetX()), static_cast<float>(print_mark12.GetY())), 10, color);

    while (ray_tracer::Matrix<4, 4>::RotationZ(M_PI / 6) * current_mark != mark_12) {
      current_mark = ray_tracer::Matrix<4, 4>::RotationZ(M_PI / 6) * current_mark;
      auto print_point = canvas_transform * current_mark;
      canvas->AddCircleFilled(
          ImVec2(static_cast<float>(print_point.GetX()), static_cast<float>(print_point.GetY())), 10, color);
      // std::cout << "Mark12:" << '\n';
      // std::cout << "x: " << current_mark.GetX() << ", y: " << current_mark.GetY() << ", z: " << current_mark.GetZ()
      // << '\n';
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
