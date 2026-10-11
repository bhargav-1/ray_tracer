#include <chrono>
#include <thread>
#include <array>
#include <iostream>

#include "imgui.h"               // NOLINT
#include "imgui_impl_glfw.h"     // NOLINT
#include "imgui_impl_opengl3.h"  // NOLINT
#define GL_SILENCE_DEPRECATION
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "app/scene_editor/add_panel.h"
#include "app/scene_editor/render_controller.h"
#include "app/scene_editor/scene_editor_app.h"
#include "app/scene_editor/scene_panel.h"
#include "app/scene_editor/render_state.h"
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

auto DrawMainLayout(SceneEditorApp& app, UiRenderState& render_state, std::array<char, 1024>& path_buffer)
    -> ray_tracer::scene_editor::ViewportPanelResult {
  bool changed = false;
  constexpr auto kFixedPaneFlags =
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(430, 900), ImGuiCond_Always);
  ImGui::Begin("Scene", nullptr, kFixedPaneFlags);
  changed |= DrawScenePanel(app, render_state, path_buffer);
  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(1410, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(190, 900), ImGuiCond_Always);
  ImGui::Begin("Add", nullptr, kFixedPaneFlags);
  changed |= DrawAddPanel(app, render_state);
  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(435, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(970, 900), ImGuiCond_Always);
  const auto viewport_result = DrawViewportPanel(app, render_state);
  return {.changed = changed || viewport_result.changed, .camera_moved = viewport_result.camera_moved};
}

}  // namespace

auto main(int argc, char** argv) -> int {
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    return 1;
  }

  const auto* glsl_version = "#version 130";
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

  SceneEditorApp app;
  UiRenderState render_state;
  std::array<char, 1024> path_buffer{};
  const bool loaded = argc > 1 ? app.LoadScene(argv[1]) : app.LoadDefaultScene();
  CopyPathToBuffer(app.State().current_path, path_buffer);
  const auto& compiled_scene = app.CompiledScene();
  if (loaded && compiled_scene.has_value()) {
    app.ClearRenderRequest();
    StartRender(render_state, *compiled_scene, false);
  }

  constexpr auto kClearColor = ImVec4(0.15F, 0.15F, 0.18F, 1.00F);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    UploadPendingTexture(render_state);
    const auto changes = DrawMainLayout(app, render_state, path_buffer);
    if (changes.changed || changes.camera_moved) {
      RequestPreviewRender(render_state, !changes.changed);
    }
    ProcessDeferredRenders(app, render_state);

    ImGui::Render();
    int display_width = 0;
    int display_height = 0;
    glfwGetFramebufferSize(window, &display_width, &display_height);
    glViewport(0, 0, display_width, display_height);
    glClearColor(kClearColor.x * kClearColor.w, kClearColor.y * kClearColor.w,
                 kClearColor.z * kClearColor.w, kClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  StopRender(render_state);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
