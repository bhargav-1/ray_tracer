// #include <imgui.h>

#include <numbers>
#include <memory>
#include "app/camera.h"
#include "app/matrix.h"
#include "app/sphere.h"
#include "app/transformations.h"
#include "app/world.h"
// #include "app/canvas.h"
#include <chrono>

#include "app/intersection.h"
#include "app/ray_tracing_engine.h"

using namespace ray_tracer;    // NOLINT
auto main() noexcept -> int {  // NOLINT

  World world{};
  auto floor = Sphere(1);
  floor.SetTransform(Matrix<4, 4>::Scaling(10, 0.01, 10));
  floor.SetMaterial(Material{Color{1, 0.9, 0.9}, 0.1, 0.9, 0.0, 200});

  auto left_wall = Sphere(2);
  left_wall.SetTransform(Matrix<4, 4>::Translation(0, 0, 5) * Matrix<4, 4>::RotationY(-std::numbers::pi / 4) *
                         Matrix<4, 4>::RotationX(std::numbers::pi / 2) * Matrix<4, 4>::Scaling(10, 0.01, 10));
  left_wall.SetMaterial(floor.GetMaterial());

  auto right_wall = Sphere(3);
  right_wall.SetTransform(Matrix<4, 4>::Translation(0, 0, 5) * Matrix<4, 4>::RotationY(std::numbers::pi / 4) *
                          Matrix<4, 4>::RotationX(std::numbers::pi / 2) * Matrix<4, 4>::Scaling(10, 0.01, 10));
  right_wall.SetMaterial(floor.GetMaterial());

  auto middle = Sphere(4);
  middle.SetTransform(Matrix<4, 4>::Translation(-0.5, 1, 0.5));
  middle.SetMaterial(Material{Color{0.1, 1, 0.5}, 0.1, 0.7, 0.3, 200});

  auto right = Sphere(5);
  right.SetTransform(Matrix<4, 4>::Translation(1.5, 0.5, -0.5) * Matrix<4, 4>::Scaling(0.5, 0.5, 0.5));
  right.SetMaterial(Material{Color{0.5, 1, 0.1}, 0.1, 0.7, 0.3, 200});

  auto left = Sphere(6);
  left.SetTransform(Matrix<4, 4>::Translation(-1.5, 0.25, -0.75) * Matrix<4, 4>::Scaling(0.1, 0.1, 0.1));
  left.SetMaterial(Material{Color{1, 0.8, 0.1}, 0.1, 0.7, 0.3, 200});

  auto left_1 = Sphere(7);
  left_1.SetTransform(Matrix<4, 4>::Translation(-1.5, 0.3, -0.75) * Matrix<4, 4>::Scaling(0.2, 0.2, 0.2));
  left_1.SetMaterial(Material{Color{1, 0.8, 0.1}, 0.1, 0.7, 0.3, 200});

  auto left_2 = Sphere(8);
  left_2.SetTransform(Matrix<4, 4>::Translation(-1.5, 0.9, -0.75) * Matrix<4, 4>::Scaling(0.4, 0.4, 0.4));
  left_2.SetMaterial(Material{Color{1, 0.8, 0.1}, 0.1, 0.7, 0.3, 200});

  auto left_3 = Sphere(8);
  left_3.SetTransform(Matrix<4, 4>::Translation(-1.5, 2.1, -0.75) * Matrix<4, 4>::Scaling(0.8, 0.8, 0.8));
  left_3.SetMaterial(Material{Color{1, 0.8, 0.1}, 0.1, 0.7, 0.3, 200});

  world.AddObject(std::make_shared<Sphere>(floor));
  world.AddObject(std::make_shared<Sphere>(left_wall));
  world.AddObject(std::make_shared<Sphere>(right_wall));
  world.AddObject(std::make_shared<Sphere>(middle));
  world.AddObject(std::make_shared<Sphere>(right));
  world.AddObject(std::make_shared<Sphere>(left));
  world.AddObject(std::make_shared<Sphere>(left_1));
  world.AddObject(std::make_shared<Sphere>(left_2));
  world.AddObject(std::make_shared<Sphere>(left_3));

  world.AddLight(PointLight(Point(-10, 10, -10), Color(1, 1, 1)));
  auto camera = Camera(1000, 500, std::numbers::pi / 3);
  camera.SetTransform(ViewTransform(Point(0, 1.5, -5), Point(0, 1, 0), Vec3(0, 1, 0)));

  auto start = std::chrono::steady_clock::now();
  auto image = Render(camera, world);
  auto end_time = std::chrono::steady_clock::now();
  std::cout << "render time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start).count() << "ms\n";
  return 0;
}
