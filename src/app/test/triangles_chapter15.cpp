#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include "app/utilities.h"
// #include "app/matrix.h"
#include "app/obj_parser.h"
#include "app/group.h"
#include "app/triangle.h"

namespace ray_tracer {
namespace {
class TemporaryObjFile {
 public:
  TemporaryObjFile(const char* filename, std::string_view contents)
      : path_(std::filesystem::temp_directory_path() / filename) {
    std::ofstream output(path_);
    output << contents;
    output.close();
    REQUIRE(output.good());
  }
  ~TemporaryObjFile() {
    std::error_code error;
    std::filesystem::remove(path_, error);
  }
  [[nodiscard]] auto Path() const -> const std::filesystem::path& { return path_; }

 private:
  std::filesystem::path path_;
};
}  // namespace

SCENARIO("15: A triangle bounding box encloses its vertices") {
  const auto triangle = Triangle(1, {Point(-3, 7, 2), Point(6, 2, -4), Point(2, -1, 5)});
  const auto bounds = triangle.BoundsOf();
  CHECK(bounds.GetMin() == Point(-3, -1, -4));
  CHECK(bounds.GetMax() == Point(6, 7, 5));
}

SCENARIO("15: Constructing a triangle") {
  GIVEN("p1 = point(0, 1, 0); p2 = point(-1, 0, 0); p3 = point(1, 0, 0); t = triangle(p1, p2, p3)") {
    auto point_1 = Point(0, 1, 0);
    auto point_2 = Point(-1, 0, 0);
    auto point_3 = Point(1, 0, 0);
    auto triangle_var = Triangle(1, {point_1, point_2, point_3});
    THEN("t.p1 == p1 and t.p2 == p2 and t.p3 == p3") {
      CHECK(triangle_var.GetVertices()[0] == point_1);
      CHECK(triangle_var.GetVertices()[1] == point_2);
      CHECK(triangle_var.GetVertices()[2] == point_3);
      CHECK(triangle_var.GetEdge1() == Vec3(-1, -1, 0));
      CHECK(triangle_var.GetEdge2() == Vec3(1, -1, 0));
      CHECK(triangle_var.GetNormal() == Vec3(0, 0, -1));
    }
  }
}
SCENARIO("15: Finding the normal on a triangle") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    THEN(
        "n1 = t.normal_at(point(0, 0.5, 0)), n2 = t.normal_at(point(-0.50, 0.75, 0)), n3 = t.normal_at(point(0.5, "
        "0.25, 0))") {
      auto normal_1 = triangle_var.NormalAt(Point(0, 0.5, 0));
      auto normal_2 = triangle_var.NormalAt(Point(-0.5, 0.75, 0));
      auto normal_3 = triangle_var.NormalAt(Point(0.5, 0.25, 0));
      CHECK(normal_1 == triangle_var.GetNormal());
      CHECK(normal_2 == triangle_var.GetNormal());
      CHECK(normal_3 == triangle_var.GetNormal());
    }
  }
}

SCENARIO("15: Intersecting a ray parallel to the triangle") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(0, -1, -2), vec(0, 1, 0))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(0, -1, -2), Vec3(0, 1, 0));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}

SCENARIO("15: A ray misses the p1-p3 edge") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(1, 1, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(1, 1, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}
SCENARIO("15: A ray misses the p1-p2 edge") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(-1, 1, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(-1, 1, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}
SCENARIO("15: A ray misses the p2-p3 edge") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(0, -1, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(0, -1, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.empty());
    }
  }
}
SCENARIO("15: A ray strikes a triangle") {
  GIVEN("t = triangle(point(0, 1, 0), point(-1, 0, 0), point(1, 0, 0)), r = ray(point(0, 0.5, -2), vec(0, 0, 1))") {
    auto triangle_var = Triangle(1, {Point(0, 1, 0), Point(-1, 0, 0), Point(1, 0, 0)});
    auto ray_var = Ray(Point(0, 0.5, -2), Vec3(0, 0, 1));
    THEN("xs = t.intersect(r)") {
      auto intersections = triangle_var.Intersect(ray_var);
      CHECK(intersections.size() == 1);
      CHECK(intersections[0].GetTime() - 2.0 < utilities::kEpsilon);
    }
  }
}

SCENARIO("15: Ignoring unrecognized lines") {
  GIVEN("gibberish -a file containing unrecognized lines") {
    THEN("the parser ignores them") {
      const auto file = TemporaryObjFile("ray_tracer_obj_gibberish.obj", R"(Vertices whisper,
Faces follow line by line.
The parser listens,
Binding points into a mesh—
Light reveals its hidden shape.
)");
      auto parsed_lines = ObjParser(file.Path().string());
      CHECK(parsed_lines.IgnoredLineCount() == 5);
    }
  }
}

SCENARIO("15: Vertex Records") {
  GIVEN("a file containing four vertex records") {
    THEN("the parser extracts the vertex records with one-based indexing") {
      const auto file = TemporaryObjFile("ray_tracer_obj_vertices.obj", R"(# Four OBJ vertices.
v -1 1 0
  v -1.0000 0.5000 0.0000

v 1 0 0
v 1 1 0
)");
      auto parsed_lines = ObjParser(file.Path().string());
      REQUIRE(parsed_lines.Vertices().size() == 5);  // Four vertices plus unused slot zero.
      CHECK(parsed_lines.Vertices()[1] == Point(-1, 1, 0));
      CHECK(parsed_lines.Vertices()[2] == Point(-1, 0.5, 0));
      CHECK(parsed_lines.Vertices()[3] == Point(1, 0, 0));
      CHECK(parsed_lines.Vertices()[4] == Point(1, 1, 0));
    }
  }
}

SCENARIO("15: Parsing triangle faces") {
  const auto file = TemporaryObjFile("ray_tracer_obj_faces.obj", R"(v -1 1 0
v -1 0 0
v 1 0 0
v 1 1 0

f 1 2 3
f 1 3 4 # second triangle
)");
  auto parser = ObjParser(file.Path().string());
  const auto& triangles = parser.Triangles();
  REQUIRE(triangles.size() == 2);
  CHECK(triangles[0].GetVertices()[0] == parser.Vertices()[1]);
  CHECK(triangles[0].GetVertices()[1] == parser.Vertices()[2]);
  CHECK(triangles[0].GetVertices()[2] == parser.Vertices()[3]);
  CHECK(triangles[1].GetVertices()[0] == parser.Vertices()[1]);
  CHECK(triangles[1].GetVertices()[1] == parser.Vertices()[3]);
  CHECK(triangles[1].GetVertices()[2] == parser.Vertices()[4]);
  const auto group = parser.Group();
  REQUIRE(group->GetChildren().size() == 2);
  const auto* child = dynamic_cast<const Triangle*>(group->GetChildren()[0].get());
  REQUIRE(child != nullptr);
  CHECK(child->GetVertices() == triangles[0].GetVertices());
  CHECK(child->GetParentGroup().lock() == group);

  CHECK(parser.Vertices().size() == 5);
  CHECK(parser.Triangles().size() == 2);
  CHECK(parser.Group()->GetChildren().size() == 2);
}

SCENARIO("15: Rejecting invalid OBJ face indices and missing files") {
  const auto file = TemporaryObjFile("ray_tracer_obj_invalid_face.obj", R"(v -1 1 0
v -1 0 0
v 1 0 0
f 0 2 3
)");
  CHECK_THROWS_AS(ObjParser(file.Path().string()), std::runtime_error);
  CHECK_THROWS_AS(ObjParser((file.Path() / "missing.obj").string()), std::runtime_error);
}

SCENARIO("15: Triangulating polygons") {
  GIVEN("a file containing v -1 1 0 v -1 00 v1 0 0 v 1 1 0 v 0 2 0 f 1 2 3 4 5") {
    THEN("the parser triangulates the polygon into triangles") {
      const auto file = TemporaryObjFile("ray_tracer_obj_polygon.obj", R"(v -1 1 0
v -1 0 0
v 1 0 0
v 1 1 0
v 0 2 0
f 1 2 3 4 5
)");
      auto parser = ObjParser(file.Path().string());
      auto group = parser.Group();
      auto triangle_1 = std::dynamic_pointer_cast<Triangle>(group->GetChildren()[0]);
        auto triangle_2 = std::dynamic_pointer_cast<Triangle>(group->GetChildren()[1]);
        auto triangle_3 = std::dynamic_pointer_cast<Triangle>(group->GetChildren()[2]);
      CHECK(triangle_1->GetVertices()[0] == parser.Vertices()[1]);
        CHECK(triangle_1->GetVertices()[1] == parser.Vertices()[2]);
        CHECK(triangle_1->GetVertices()[2] == parser.Vertices()[3]);
        CHECK(triangle_2->GetVertices()[0] == parser.Vertices()[1]);
        CHECK(triangle_2->GetVertices()[1] == parser.Vertices()[3]);
        CHECK(triangle_2->GetVertices()[2] == parser.Vertices()[4]);
        CHECK(triangle_3->GetVertices()[0] == parser.Vertices()[1]);
        CHECK(triangle_3->GetVertices()[1] == parser.Vertices()[4]);
        CHECK(triangle_3->GetVertices()[2] == parser.Vertices()[5]);
    }
  }
}

SCENARIO("15: Triangles in groups") {
  GIVEN("a file  triangles.obj, v -1 1 0 v -1 0 0 v 1 0 0 v 1 1 0 g FirstGroup f 1 2 3 g SecondGroup f 1 3 4") {
    THEN("the parser creates groups for the triangles") {
      const auto file = TemporaryObjFile("ray_tracer_obj_triangles.obj", R"(v -1 1 0
v -1 0 0
v 1 0 0
v 1 1 0
g FirstGroup
f 1 2 3
g SecondGroup
f 1 3 4
)");
        auto parser = ObjParser(file.Path().string());
        auto group = parser.Group();
        REQUIRE(group->GetChildren().size() == 2);
        auto first_group = std::dynamic_pointer_cast<Group>(group->GetChildren()[0]);
        auto second_group = std::dynamic_pointer_cast<Group>(group->GetChildren()[1]);
        REQUIRE(first_group != nullptr);
        REQUIRE(second_group != nullptr);
        CHECK(first_group->GetChildren().size() == 1);
        CHECK(second_group->GetChildren().size() == 1);
        auto triangle_1 = std::dynamic_pointer_cast<Triangle>(first_group->GetChildren()[0]);
        auto triangle_2 = std::dynamic_pointer_cast<Triangle>(second_group->GetChildren()[0]);
        REQUIRE(triangle_1 != nullptr);
        REQUIRE(triangle_2 != nullptr);
        CHECK(triangle_1->GetVertices()[0] == parser.Vertices()[1]);
        CHECK(triangle_1->GetVertices()[1] == parser.Vertices()[2]);
        CHECK(triangle_1->GetVertices()[2] == parser.Vertices()[3]);
        CHECK(triangle_2->GetVertices()[0] == parser.Vertices()[1]);
        CHECK(triangle_2->GetVertices()[1] == parser.Vertices()[3]);
        CHECK(triangle_2->GetVertices()[2] == parser.Vertices()[4]);
    }
  }
}

SCENARIO("15: Converting an OBJ file to a group") {
        GIVEN("a file triangles.obj, v -1 1 0 v -1 0 0 v 1 0 0 v 1 1 0 g FirstGroup f 1 2 3 g SecondGroup f 1 3 4") {
        THEN("the parser creates a group containing the triangles") {
        const auto file = TemporaryObjFile("ray_tracer_obj_triangles.obj", R"(v -1 1 0
v -1 0 0
v 1 0 0
v 1 1 0
g FirstGroup
f 1 2 3
g SecondGroup
f 1 3 4
)");
        auto parser = ObjParser(file.Path().string());
        auto group = parser.ObjToGroup();
        REQUIRE(group->GetChildren().size() == 2);
        const auto& children = group->GetChildren();
        CHECK(std::ranges::find(children, parser.NamedGroup("FirstGroup")) != children.end());
        CHECK(std::ranges::find(children, parser.NamedGroup("SecondGroup")) != children.end());
        CHECK(parser.NamedGroup("FirstGroup")->GetParentGroup().lock() == group);
        CHECK(parser.NamedGroup("SecondGroup")->GetParentGroup().lock() == group);
    }
  }
}

}  // namespace ray_tracer
