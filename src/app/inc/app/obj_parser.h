#pragma once

#include <array>
#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "triangle.h"
#include "group.h"
#include "point.h"

namespace ray_tracer {
class ObjParser {
 public:
  explicit ObjParser(const std::string& filename, std::size_t first_shape_id = 0)
      : filename_(filename), file_(filename), first_shape_id_(first_shape_id) {
    if (!file_.is_open()) {
      throw std::runtime_error("Failed to open file: " + filename);
    }
    Parse();
  }
  [[nodiscard]] auto Group() const -> std::shared_ptr<ray_tracer::Group> { return group_; }
  [[nodiscard]] auto ObjToGroup() const -> std::shared_ptr<ray_tracer::Group> { return group_; }
  [[nodiscard]] auto NamedGroup(const std::string& name) const -> std::shared_ptr<ray_tracer::Group> {
    return named_groups_.at(name);
  }
  // Slot zero is unused: OBJ vertex indices start at one.
  [[nodiscard]] auto Vertices() const -> const std::vector<Point>& { return vertices_; }
  [[nodiscard]] auto Triangles() const -> const std::vector<Triangle>& { return triangles_; }
  [[nodiscard]] auto IgnoredLineCount() const -> std::size_t { return ignored_line_count_; }

 private:
  auto Parse() -> void;
  auto FanTriangulateFace(const std::vector<long>& indices) -> void;
  std::string filename_;
  std::ifstream file_;
  std::size_t first_shape_id_ = 0;
  std::vector<Point> vertices_{Point(0, 0, 0)};
  std::vector<Triangle> triangles_;
  std::shared_ptr<ray_tracer::Group> group_{std::make_shared<ray_tracer::Group>(0)};
  std::shared_ptr<ray_tracer::Group> current_group_;
  std::unordered_map<std::string, std::shared_ptr<ray_tracer::Group>> named_groups_;
  std::size_t next_shape_id_ = 1;
  std::size_t ignored_line_count_ = 0;
};

inline auto ObjParser::Parse() -> void {
  file_.clear();
  file_.seekg(0);
  vertices_ = {Point(0, 0, 0)};
  triangles_.clear();
  group_ = std::make_shared<ray_tracer::Group>(first_shape_id_);
  current_group_ = group_;
  named_groups_.clear();
  next_shape_id_ = first_shape_id_ + 1;
  ignored_line_count_ = 0;

  std::string line;
  std::size_t line_number = 0;
  while (std::getline(file_, line)) {
    ++line_number;
    std::istringstream record(line);
    std::string type;
    if (!(record >> type) || type.starts_with('#')) {
      continue;
    }
    const auto invalid_record = [&](const char* message) {
      throw std::runtime_error(filename_ + ":" + std::to_string(line_number) + ": " + message);
    };
    if (type == "v") {
      double x_coordinate = 0;
      double y_coordinate = 0;
      double z_coordinate = 0;
      if (!(record >> x_coordinate >> y_coordinate >> z_coordinate)) {
        invalid_record("expected vertex coordinates: v x y z");
      }
      vertices_.emplace_back(x_coordinate, y_coordinate, z_coordinate);
    } else if (type == "g") {
      std::string name;
      if (!(record >> name) || name.starts_with('#')) {
        current_group_ = group_;
        continue;
      }
      auto& named_group = named_groups_[name];
      if (!named_group) {
        named_group = std::make_shared<ray_tracer::Group>(next_shape_id_++);
        group_->AddChild(named_group);
      }
      current_group_ = named_group;
    } else if (type == "f") {
      std::vector<long> indices;
      while (record >> std::ws && record.peek() != '#' && !record.eof()) {
        long index = 0;
        if (!(record >> index)) {
          invalid_record("expected a plain vertex index");
        }
        indices.push_back(index);
      }
      if (indices.size() < 3) {
        invalid_record("expected a face with at least three vertex indices");
      }
      for (const auto index : indices) {
        if (index <= 0 || static_cast<std::size_t>(index) >= vertices_.size()) {
          invalid_record("face index must reference an existing vertex, starting at 1");
        }
      }
      if (indices.size() == 3) {
        const std::array points{vertices_[indices[0]], vertices_[indices[1]], vertices_[indices[2]]};
        triangles_.emplace_back(next_shape_id_++, points);
        current_group_->AddChild(std::make_shared<Triangle>(triangles_.back()));
      } else {
        FanTriangulateFace(indices);
      }
    } else {
      ++ignored_line_count_;
    }
  }
  if (file_.bad()) {
    throw std::runtime_error("Failed to read file: " + filename_);
  }
}

inline auto ObjParser::FanTriangulateFace(const std::vector<long>& indices) -> void {
  for (std::size_t i = 1; i + 1 < indices.size(); ++i) {
    const std::array points{vertices_[indices[0]], vertices_[indices[i]], vertices_[indices[i + 1]]};
    triangles_.emplace_back(next_shape_id_++, points);
    current_group_->AddChild(std::make_shared<Triangle>(triangles_.back()));
  }
}

}  // namespace ray_tracer
