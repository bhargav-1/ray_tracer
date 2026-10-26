#pragma once

#include <array>
#include <cstddef>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "app/primitives/point.h"
#include "app/primitives/vec3.h"
#include "app/shapes/group.h"
#include "app/shapes/triangle.h"

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
  [[nodiscard]] auto Triangles() const -> const std::vector<std::shared_ptr<Triangle>>& { return triangles_; }
  [[nodiscard]] auto GetNormals() const -> const std::vector<Vec3>& { return normals_; }
  [[nodiscard]] auto IgnoredLineCount() const -> std::size_t { return ignored_line_count_; }

 private:
  auto Parse() -> void;
  auto FanTriangulateFace(const std::vector<long>& indices, const std::vector<long>& normal_indices) -> void;
  std::string filename_;
  std::ifstream file_;
  std::size_t first_shape_id_ = 0;
  std::vector<Point> vertices_{Point(0, 0, 0)};
  std::vector<std::shared_ptr<Triangle>> triangles_;
  std::shared_ptr<ray_tracer::Group> group_{std::make_shared<ray_tracer::Group>(0)};
  std::shared_ptr<ray_tracer::Group> current_group_;
  std::unordered_map<std::string, std::shared_ptr<ray_tracer::Group>> named_groups_;
  std::size_t next_shape_id_ = 1;
  std::size_t ignored_line_count_ = 0;
  std::vector<Vec3> normals_{Vec3(0, 0, 0)};
};

inline auto ObjParser::Parse() -> void {
  file_.clear();
  file_.seekg(0);
  vertices_ = {Point(0, 0, 0)};
  normals_ = {Vec3(0, 0, 0)};
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
    if (type == "v" || type == "vn") {
      double x_coordinate = 0;
      double y_coordinate = 0;
      double z_coordinate = 0;
      if (!(record >> x_coordinate >> y_coordinate >> z_coordinate)) {
        invalid_record(type == "v" ? "expected vertex coordinates: v x y z" : "expected normal coordinates: vn x y z");
      }
      if (type == "v") {
        vertices_.emplace_back(x_coordinate, y_coordinate, z_coordinate);
      } else {
        normals_.emplace_back(x_coordinate, y_coordinate, z_coordinate);
      }
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
      std::vector<long> normal_indices;
      std::string token;
      while (record >> token) {
        if (token.starts_with('#')) {
          break;
        }
        std::istringstream entry(token);
        long vertex_index = 0;
        long normal_index = 0;
        if (!(entry >> vertex_index)) {
          invalid_record("expected a vertex index");
        }
        if (entry.peek() == '/') {
          entry.get();
          // Skip the unused texture index, including an empty one in v//vn.
          entry.ignore(std::numeric_limits<std::streamsize>::max(), '/');
          if (!entry.eof()) {
            if (!(entry >> normal_index)) {
              invalid_record("expected a normal index after the second slash");
            }
            if (normal_index <= 0 || static_cast<std::size_t>(normal_index) >= normals_.size()) {
              invalid_record("face index must reference an existing normal, starting at 1");
            }
          }
        }
        if (!entry.eof() && entry.peek() != std::char_traits<char>::eof()) {
          invalid_record("unexpected characters in face index");
        }
        if (vertex_index <= 0 || static_cast<std::size_t>(vertex_index) >= vertices_.size()) {
          invalid_record("face index must reference an existing vertex, starting at 1");
        }
        indices.push_back(vertex_index);
        normal_indices.push_back(normal_index);
      }
      if (indices.size() < 3) {
        invalid_record("expected a face with at least three vertex indices");
      }
      for (const auto normal_index : normal_indices) {
        if ((normal_index == 0) != (normal_indices[0] == 0)) {
          invalid_record("all vertices of a face must either include normals or omit them");
        }
      }
      FanTriangulateFace(indices, normal_indices);
    } else {
      ++ignored_line_count_;
    }
  }
  if (file_.bad()) {
    throw std::runtime_error("Failed to read file: " + filename_);
  }
}

inline auto ObjParser::FanTriangulateFace(const std::vector<long>& indices,
                                          const std::vector<long>& normal_indices) -> void {
  for (std::size_t index = 1; index + 1 < indices.size(); ++index) {
    const std::array points{vertices_[indices[0]], vertices_[indices[index]], vertices_[indices[index + 1]]};
    std::shared_ptr<Triangle> triangle;
    if (normal_indices[0] != 0) {
      const std::array normals{
          normals_[normal_indices[0]], normals_[normal_indices[index]], normals_[normal_indices[index + 1]]};
      triangle = std::make_shared<SmoothTriangle>(next_shape_id_++, points, normals);
    } else {
      triangle = std::make_shared<Triangle>(next_shape_id_++, points);
    }
    triangles_.push_back(triangle);
    current_group_->AddChild(triangle);
  }
}

}  // namespace ray_tracer
