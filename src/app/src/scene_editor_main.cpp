#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <thread>

#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "app/scene_editor/add_panel.h"
#include "app/scene_editor/render_controller.h"
#include "app/scene_editor/render_state.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/scene_panel.h"
#include "app/scene_editor/viewport_panel.h"

namespace {

using ray_tracer::scene_editor::CopyPathToBuffer;
using ray_tracer::scene_editor::DrawAddPanel;
using ray_tracer::scene_editor::DrawScenePanel;
using ray_tracer::scene_editor::DrawViewportPanel;
using ray_tracer::scene_editor::ProcessDeferredRenders;
using ray_tracer::scene_editor::RequestPreviewRender;
using ray_tracer::scene_editor::SceneEditorApp;
using ray_tracer::scene_editor::StartRender;
using ray_tracer::scene_editor::StopRender;
using ray_tracer::scene_editor::UiRenderState;
using ray_tracer::scene_editor::UploadPendingTexture;

auto GlfwErrorCallback(int error, const char* description) -> void {
  std::cerr << "GLFW Error " << error << ": " << description << '\n';
}

auto DrawMainLayout(SceneEditorApp& app,
                    UiRenderState& render_state,
                    std::array<char, 1024>& path_buffer) -> ray_tracer::scene_editor::ViewportPanelResult {
  bool changed = false;
  constexpr auto kSidePaneFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
  static bool scene_collapsed = false;
  static bool add_collapsed = false;

  const auto size = ImGui::GetIO().DisplaySize;
  const float expanded_left_width = std::min(430.0F, size.x * 0.27F);
  const float expanded_right_width = std::min(190.0F, size.x * 0.12F);
  const float collapsed_left_width = std::min(110.0F, expanded_left_width);
  const float collapsed_right_width = std::min(70.0F, expanded_right_width);
  float left_width = scene_collapsed ? collapsed_left_width : expanded_left_width;
  float right_width = add_collapsed ? collapsed_right_width : expanded_right_width;
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(left_width, size.y), ImGuiCond_Always);
  const bool scene_open = ImGui::Begin("Scene", nullptr, kSidePaneFlags);
  scene_collapsed = !scene_open;
  left_width = scene_collapsed ? collapsed_left_width : expanded_left_width;
  ImGui::SetWindowSize(ImVec2(left_width, size.y), ImGuiCond_Always);
  if (scene_open) {
    changed |= DrawScenePanel(app, render_state, path_buffer);
  }
  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(size.x - right_width, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(right_width, size.y), ImGuiCond_Always);
  const bool add_open = ImGui::Begin("Add", nullptr, kSidePaneFlags);
  add_collapsed = !add_open;
  right_width = add_collapsed ? collapsed_right_width : expanded_right_width;
  ImGui::SetWindowPos(ImVec2(size.x - right_width, 0), ImGuiCond_Always);
  ImGui::SetWindowSize(ImVec2(right_width, size.y), ImGuiCond_Always);
  if (add_open) {
    changed |= DrawAddPanel(app, render_state);
  }
  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(left_width + 5.0F, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(std::max(1.0F, size.x - left_width - right_width - 10.0F), size.y), ImGuiCond_Always);
  const auto viewport_result = DrawViewportPanel(app, render_state);
  return {.changed = changed || viewport_result.changed, .camera_moved = viewport_result.camera_moved};
}

}  // namespace

auto main(int argc, char** argv) -> int {
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    return 1;
  }

#ifdef __EMSCRIPTEN__
  const auto* glsl_version = "#version 300 es";
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
  const auto* glsl_version = "#version 130";
#endif
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(1600, 900, "Scene Editor", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& imgui_io = ImGui::GetIO();
  imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  imgui_io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Browser callbacks outlive main's stack; keep their editor state alive.
  static SceneEditorApp app;
  static UiRenderState render_state;
  static std::array<char, 1024> path_buffer{};
  const bool loaded = argc > 1 ? app.LoadScene(argv[1]) : app.LoadDefaultScene();
  CopyPathToBuffer(app.State().current_path, path_buffer);
  const auto& compiled_scene = app.CompiledScene();
  if (loaded && compiled_scene.has_value()) {
    app.ClearRenderRequest();
    StartRender(render_state, *compiled_scene, false);
  }

  static auto draw_frame = [window]() {
    constexpr auto kClearColor = ImVec4(0.15F, 0.15F, 0.18F, 1.00F);
#ifdef __EMSCRIPTEN__
    double width = 0;
    double height = 0;
    emscripten_get_element_css_size("#canvas", &width, &height);
    int old_width = 0;
    int old_height = 0;
    glfwGetWindowSize(window, &old_width, &old_height);
    if (width > 0 && height > 0 && (old_width != static_cast<int>(width) || old_height != static_cast<int>(height))) {
      glfwSetWindowSize(window, static_cast<int>(width), static_cast<int>(height));
    }
#endif
    glfwPollEvents();
#ifndef __EMSCRIPTEN__
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
      return;
    }
#endif

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    UploadPendingTexture(render_state);
    const auto changes = DrawMainLayout(app, render_state, path_buffer);
    render_state.interaction_active = (ImGui::IsAnyItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) ||
                                      ImGui::IsMouseDragging(ImGuiMouseButton_Left) ||
                                      ImGui::IsMouseDragging(ImGuiMouseButton_Right);
    if (changes.changed || changes.camera_moved) {
      RequestPreviewRender(render_state, !changes.changed);
    }
    ProcessDeferredRenders(app, render_state);

    ImGui::Render();
    int display_width = 0;
    int display_height = 0;
    glfwGetFramebufferSize(window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);
    glClearColor(
        kClearColor.x * kClearColor.w, kClearColor.y * kClearColor.w, kClearColor.z * kClearColor.w, kClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  };

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(
      [](void* frame) { (*static_cast<decltype(draw_frame)*>(frame))(); }, &draw_frame, 0, true);
#else
  while (!glfwWindowShouldClose(window)) {
    draw_frame();
  }
#endif

  StopRender(render_state);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
