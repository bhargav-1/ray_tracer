#pragma once

#include <filesystem>
#include <string>

class ImGuiFileDialog {
 public:
  static auto Instance() -> ImGuiFileDialog*;

  auto OpenDialog(const std::string& key,
                  const std::string& title,
                  const std::string& filters,
                  const std::filesystem::path& start_path) -> void;
  auto OpenDialog(const std::string& key,
                  const std::string& title,
                  const std::string& filters,
                  const std::filesystem::path& start_path,
                  const std::string& accept_label) -> void;
  auto Display(const std::string& key) -> bool;
  [[nodiscard]] auto IsOk() const -> bool { return accepted_; }
  [[nodiscard]] auto GetFilePathName() const -> std::string { return selected_path_.string(); }
  auto Close() -> void;

 private:
  auto RefreshEntries() -> void;
  auto AcceptSelection() -> void;

  std::string key_;
  std::string title_;
  std::string filters_;
  std::string accept_label_{"Load"};
  std::filesystem::path current_directory_;
  std::filesystem::path selected_path_;
  std::string typed_filename_;
  bool open_{false};
  bool accepted_{false};
  std::string error_;
};
