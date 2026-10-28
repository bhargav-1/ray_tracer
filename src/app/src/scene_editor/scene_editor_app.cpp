#include "app/asset_directory.h"

#include "app/scene_editor/scene_editor_app.h"

#include <cstddef>
#include <filesystem>
#include <utility>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app/scene/scene.h"
#include "app/scene/scene_compiler.h"
#include "app/scene/scene_yaml.h"

namespace ray_tracer::scene_editor {

auto SceneEditorApp::LoadDefaultScene() -> bool {
  return LoadScene(AssetDirectory() / "scenes" / "chapter_11_demo.yaml");
}

auto SceneEditorApp::LoadScene(const std::filesystem::path& path) -> bool {
  auto loaded = scene_io::LoadSceneYaml(path);
  if (!loaded.ok) {
    state_.status_message = loaded.error;
    return false;
  }
  auto previous_scene = current_scene_;
  auto previous_compiled = compiled_scene_;
  current_scene_ = std::move(loaded.scene);
  if (!CompileCurrentScene()) {
    current_scene_ = std::move(previous_scene);
    compiled_scene_ = std::move(previous_compiled);
    return false;
  }
  state_.selection = Selection{};
  state_.current_path = path;
  state_.status_message = "Loaded " + path.string();
  return true;
}

auto SceneEditorApp::SaveScene(const std::filesystem::path& path) -> bool {
  const auto saved = scene_io::SaveSceneYaml(current_scene_, path);
  if (!saved.ok) {
    state_.status_message = saved.error;
    return false;
  }
  state_.current_path = path;
  state_.status_message = "Saved " + path.string();
#ifdef __EMSCRIPTEN__
  // The browser filesystem is temporary; Save As also downloads the YAML.
  // clang-format off: the macro body is JavaScript, not C++.
  EM_ASM({
    const path = UTF8ToString($0);
    const url = URL.createObjectURL(new Blob([FS.readFile(path)], {type: 'application/yaml'}));
    const link = document.createElement('a');
    link.href = url;
    link.download = path.split('/').pop();
    link.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }, path.string().c_str());
  // clang-format on
#endif
  return true;
}

auto SceneEditorApp::SetCurrentScene(scene::Scene scene) -> bool {
  auto previous_scene = current_scene_;
  auto previous_compiled = compiled_scene_;
  current_scene_ = std::move(scene);
  if (!CompileCurrentScene()) {
    current_scene_ = std::move(previous_scene);
    compiled_scene_ = std::move(previous_compiled);
    return false;
  }
  state_.selection = Selection{};
  return true;
}

auto SceneEditorApp::AddObj(const std::filesystem::path& path, const std::vector<std::size_t>& parent_path) -> bool {
  auto candidate = current_scene_;
  auto* destination = &candidate.objects;
  for (const auto index : parent_path) {
    if (index >= destination->size() || !(*destination)[index].IsContainer()) {
      state_.status_message = "OBJ import destination is no longer a group";
      return false;
    }
    destination = &(*destination)[index].Children();
  }
  scene::Object object;
  object.name = path.stem().string();
  object.Shape().type = scene::ObjectType::kObj;
  object.Shape().source = std::filesystem::absolute(path).lexically_normal().string();
  destination->push_back(std::move(object));
  auto selection_path = parent_path;
  selection_path.push_back(destination->size() - 1);
  if (!SetCurrentScene(std::move(candidate))) {
    return false;
  }
  state_.selection = Selection::ForObject(std::move(selection_path));
  state_.status_message = "Added OBJ " + path.string();
  return true;
}

auto SceneEditorApp::RecompileCurrentScene() -> bool { return CompileCurrentScene(); }

auto SceneEditorApp::UpdateCamera() -> bool {
  if (!compiled_scene_.has_value() || !current_scene_.camera.has_value()) {
    return RecompileCurrentScene();
  }
  compiled_scene_->camera = scene_runtime::CompileCamera(*current_scene_.camera);
  return true;
}

auto SceneEditorApp::CompileCurrentScene() -> bool {
  auto compiled = scene_runtime::CompileScene(current_scene_);
  if (!compiled.ok || !compiled.compiled.has_value()) {
    state_.status_message = compiled.error;
    return false;
  }
  compiled_scene_ = std::move(compiled.compiled);
  state_.render_state.render_requested = true;
  return true;
}

}  // namespace ray_tracer::scene_editor
