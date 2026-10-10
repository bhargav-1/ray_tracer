#include "app/scene_editor/imgui_file_dialog.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <exception>
#include <filesystem>
#include <string>
#include <sstream>
#include <vector>

#include "imgui.h"  // NOLINT

namespace {

struct FileDialogEntry {
  std::filesystem::path path;
  bool directory{false};
};

auto HasAllowedExtension(const std::filesystem::path& path, const std::string& filters) -> bool {
  std::istringstream extensions(filters);
  std::string extension;
  while (std::getline(extensions, extension, ',')) {
    if (path.extension() == extension) {
      return true;
    }
  }
  return false;
}

auto CopyToBuffer(const std::string& text, std::array<char, 1024>& buffer) -> void {
  buffer.fill('\0');
  const auto count = std::min(text.size(), buffer.size() - 1);
  std::memcpy(buffer.data(), text.data(), count);
}

auto SortedEntries(const std::filesystem::path& directory, const std::string& filters,
                   std::string& error) -> std::vector<FileDialogEntry> {
  std::vector<FileDialogEntry> entries;
  error.clear();
  try {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      const bool is_directory = entry.is_directory();
      if (!is_directory && !HasAllowedExtension(entry.path(), filters)) {
        continue;
      }
      entries.push_back({entry.path(), is_directory});
    }
  } catch (const std::exception& exception) {
    error = exception.what();
  }
  std::ranges::sort(entries, [](const FileDialogEntry& lhs, const FileDialogEntry& rhs) {
    if (lhs.directory != rhs.directory) {
      return lhs.directory && !rhs.directory;
    }
    return lhs.path.filename().string() < rhs.path.filename().string();
  });
  return entries;
}

}  // namespace

auto ImGuiFileDialog::Instance() -> ImGuiFileDialog* {
  static ImGuiFileDialog dialog;
  return &dialog;
}

auto ImGuiFileDialog::OpenDialog(const std::string& key,
                                 const std::string& title,
                                 const std::string& filters,
                                 const std::filesystem::path& start_path) -> void {
  OpenDialog(key, title, filters, start_path, "Load");
}

auto ImGuiFileDialog::OpenDialog(const std::string& key,
                                 const std::string& title,
                                 const std::string& filters,
                                 const std::filesystem::path& start_path,
                                 const std::string& accept_label) -> void {
  key_ = key;
  title_ = title;
  filters_ = filters;
  accept_label_ = accept_label;
  accepted_ = false;
  error_.clear();

  std::filesystem::path start = start_path.empty() ? std::filesystem::current_path() : start_path;
  if (std::filesystem::is_regular_file(start)) {
    selected_path_ = start;
    typed_filename_ = start.filename().string();
    start = start.parent_path();
  } else if (!selected_path_.empty()) {
    typed_filename_ = selected_path_.filename().string();
  }
  current_directory_ = std::filesystem::absolute(start);
  open_ = true;
  ImGui::OpenPopup(key_.c_str());
}

auto ImGuiFileDialog::Display(const std::string& key) -> bool {
  if (!open_ || key != key_) {
    return false;
  }

  bool should_close = false;
  ImGui::SetNextWindowSize(ImVec2(720.0F, 520.0F), ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal(key_.c_str(), &open_)) {
    ImGui::TextUnformatted(title_.c_str());
    ImGui::TextWrapped("%s", current_directory_.string().c_str());
    if (!filters_.empty()) {
      ImGui::TextDisabled("%s", filters_.c_str());
    }

    if (ImGui::Button("Up") && current_directory_.has_parent_path()) {
      current_directory_ = current_directory_.parent_path();
    }
    ImGui::SameLine();
    if (ImGui::Button("Assets")) {
      current_directory_ = std::filesystem::path{RAY_TRACER_ASSET_DIR} / "scenes";
    }
    ImGui::Separator();

    auto entries = SortedEntries(current_directory_, filters_, error_);
    if (!error_.empty()) {
      ImGui::TextWrapped("%s", error_.c_str());
    }

    if (ImGui::BeginChild("file_dialog_entries", ImVec2(0.0F, -78.0F), true)) {
      for (const auto& entry : entries) {
        const auto filename = entry.path.filename().string();
        const std::string label = entry.directory ? "[dir] " + filename : filename;
        const bool selected = entry.path == selected_path_;
        if (ImGui::Selectable(label.c_str(), selected, entry.directory ? ImGuiSelectableFlags_DontClosePopups : 0)) {
          if (entry.directory) {
            current_directory_ = entry.path;
          } else {
            selected_path_ = entry.path;
            typed_filename_ = filename;
          }
        }
      }
    }
    ImGui::EndChild();

    std::array<char, 1024> filename_buffer{};
    CopyToBuffer(typed_filename_, filename_buffer);
    if (ImGui::InputText("File", filename_buffer.data(), filename_buffer.size())) {
      typed_filename_ = filename_buffer.data();
      selected_path_ = current_directory_ / typed_filename_;
    }

    if (ImGui::Button(accept_label_.c_str(), ImVec2(120.0F, 0.0F))) {
      AcceptSelection();
      should_close = accepted_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120.0F, 0.0F))) {
      accepted_ = false;
      should_close = true;
    }

    if (!error_.empty()) {
      ImGui::SameLine();
      ImGui::TextWrapped("%s", error_.c_str());
    }

    if (should_close) {
      Close();
    }
    ImGui::EndPopup();
  } else if (!open_) {
    return true;
  }

  return should_close;
}

auto ImGuiFileDialog::Close() -> void {
  open_ = false;
  ImGui::CloseCurrentPopup();
}

auto ImGuiFileDialog::AcceptSelection() -> void {
  if (!typed_filename_.empty()) {
    selected_path_ = current_directory_ / typed_filename_;
  }
  if (selected_path_.empty()) {
    error_ = "Select a file matching " + filters_;
    accepted_ = false;
    return;
  }
  if (accept_label_ == "Save" && !selected_path_.has_extension()) {
    selected_path_ += ".yaml";
  }
  if (!HasAllowedExtension(selected_path_, filters_)) {
    error_ = "Select a file matching " + filters_;
    accepted_ = false;
    return;
  }
  accepted_ = true;
  error_.clear();
}

auto ImGuiFileDialog::RefreshEntries() -> void {}
