# Ray Tracer Challenge

[![Linux CI](https://github.com/bhargav-1/ray_tracer/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/bhargav-1/ray_tracer/actions/workflows/ci.yml)

Yet another implementation of [*The Ray Tracer Challenge*](http://raytracerchallenge.com/)
by Jamis Buck, written in C++23. I set out to build a ray tracer from scratch
while following the book as closely as possible.

[Try in browser](https://bhargav-1.github.io/ray_tracer/)

![Starting with solid shapes, editing colors and patterns, enabling reflections, stretching a sphere, and orbiting the camera](images/scene_editor_demo.gif)

Along the way, I wanted an easier way to visualize the results and experiment
with materials, transforms, cameras, and lights. That led to a scene editor
built with Dear ImGui, GLFW, and OpenGL. Ray tracing runs on the CPU; the editor
displays the rendered image and lets me change the scene interactively.

It started as a curiosity to play with and understand ray tracing concepts. It
has been immensely satisfying and incredibly fun to experiment with and explore
them. I started out thinking I could claim the world of graphics with just this
book, but I quickly realized I am only scratching the surface and have a lot to learn.

## Build and run

The project targets Linux, with Ubuntu 24.04 as the Docker base and reference host environment.

### Docker

With Docker available, install Invoke in your Python environment:

```sh
pip install invoke
# Alternatively: uv pip install invoke
```

Build the development image, start the container, and open its shell:

```sh
invoke docker.build
invoke docker.up
invoke docker.login
```

Inside the container, build and launch the scene editor:

```sh
invoke app.build
invoke app.run
```

Browser build and local server: `invoke app.build.web` then `invoke app.run.web`.

The container uses the standard runtime by default. Use `invoke docker.up --gpu` to enable the NVIDIA runtime for GPU-accelerated OpenGL display; this requires the NVIDIA Container Toolkit. Ray tracing still runs on the CPU. `invoke docker.login` connects to `localhost:1122`.

### Install the Ubuntu package

For Ubuntu 24.04 x86_64, download the `.deb` from [Releases](https://github.com/bhargav-1/ray_tracer/releases), install it with `sudo apt install ./raytracerchallenge_*.deb`, and run `raytracerchallenge [scene.yaml]`.

## Exploring scenes

Load one of the [scene files](assets/scenes), import an OBJ model, or build a scene
in the editor. You can change materials, transforms, cameras, and lights, then
render the result. Model paths are saved relative to the scene file.

To open a scene directly:

```sh
invoke app.run --args="assets/scenes/chapter_16_rounded_csg_dice.yaml"
```

## Chapters and results

Results are shown from the later chapters back to the first visual examples. Chapters 1 and 3 cover tuples and matrices; they have tests but no standalone visual demos.

### Chapter 16: Constructive solid geometry

CSG combines shapes using union, intersection, and difference. The dice use these operations to create rounded edges and recessed pips.

![Three rounded dice with recessed pips](images/chapter_16_csg.gif)

### Chapter 15: Triangles and OBJ models

OBJ loading makes it possible to render triangle meshes. This comparison shows the same teapot without normals and with normals interpolation.

![Teapots rendered with flat shading on the left and smooth shading on the right](images/chapter_15_triangles_obj.gif)

### Chapter 14: Groups

Groups let related objects share transforms and form larger structures.

![Hexagonal structure built from grouped objects](images/chapter_14_groups.gif)

### Chapter 13: Cylinders and cones

This [material study](assets/scenes/chapter_13_cylinders_cones.yaml) combines glass and mirror cylinders, stepped brass rings, and cones with matte cobalt, polished copper, and glazed emerald finishes on a checkerboard floor.

![Glass and mirror cylinders, stepped rings, and three cones with different materials on a checkerboard floor](images/chapter_13_material_study.gif)

### Chapter 12: Cubes

A furnished room built largely from transformed cubes.

![Room with a table, framed panels, and cubes](images/chapter_12_room_cubes.gif)

### Chapter 11: Reflection and refraction

The renderer supports reflective and transparent materials, refraction, and Schlick's approximation.

![Pond and rocks displayed in the scene editor](images/chapter_11_reflections.gif)

### Chapter 10: Patterns

Procedural patterns can be transformed, nested, and blended.
(Notice the fixed checker pattern on the sphere; other shapes can be handled as
explained in the bonus chapter on textures.)

![Spheres and a floor with procedural patterns](images/chapter_10_patterns.png)

Perturbing a pattern with noise produces less regular surfaces.

![Perturbed patterns on green spheres and a floor](images/chapter_10_perturbed_patterns.png)

### Chapter 9: Planes

Planes form a room containing transformed spheres.
I played around again and generated a very odd-looking result.

<img src="images/chapter_09_planes.png" alt="Colorful room made from planes with spheres on its surfaces" width="400">

### Chapter 8: Shadows

An eclipse example built with shadow rays.

<img src="images/chapter_08_shadows.png" alt="Eclipse rendered using shadow rays" width="400">

### Chapter 7: Making a scene

A camera, materials, and multiple objects come together in a scene.
Both the GUI example and the PPM renderer produce visible output.
I played around with transforms to get a very odd-looking result.

<img src="images/chapter_07_scene.png" alt="Chapter 7 scene displayed in ImGui" width="400">

<img src="images/chapter_07_scene_ppm.png" alt="PNG exported from the chapter 7 PPM renderer" width="400">

### Chapter 6: Light and shading

Lighting gives the sphere its shape, shading, and specular highlight.

<img src="images/chapter_06_lighting.png" alt="Blue sphere with lighting and a specular highlight" width="400">

### Chapter 5: Ray-sphere intersections

Casting rays onto a transformed sphere produces its silhouette.

<img src="images/chapter_05_ray_sphere.png" alt="Red silhouette of a transformed sphere" width="400">

### Chapter 4: Transformations

A clock face built by rotating and translating points.

<img src="images/chapter_04_clock.png" alt="Clock face showing transformed points" width="400">

### Chapter 2: Drawing on a canvas

The projectile example writes a PPM image. This PNG is an export of that output.

<img src="images/chapter_02_projectile.png" alt="Projectile trajectory drawn on a canvas" width="400">

## Bonus chapters

- **Bounding boxes and bounding volume hierarchies (BVH): implemented.**
   The [dragon scene](assets/scenes/bonus_bvh_dragons.yaml) and [sphere grid](assets/scenes/bonus_bvh_spheres_5x5x5.yaml)
   provide larger scenes to explore. This really helps reduce render times for complex scenes.

  ![Dragon models with transparent bounding boxes](images/bonus_bounding_boxes_dragon.png)

- **Soft shadows:** pending.
- **Image textures:** pending.

## Lessons learned

- I think the most common theme with numbers was that `==` does not mean anything for floating point values; it's always about `~=` approximations.
  Small differences can result in salt and pepper noise in the final image, so I had to be careful with floating point comparisons.
- CPUs are slow when crunching large amounts of data; I might have to play with a GPU port for better numbers (real time!?).
- Always cache the results of expensive calculations if they are reused.
    (This saved render time many times over.)
- Multithreading can do wonders.
- Build flags can change rendering times, but optimized builds can make debugging a
  pain because variables may be missing in the debugger. I prefer `-Og -g` for debugging and `-O3` for release builds. `-g` adds debug symbols; the `-O` flag controls optimization.

    Same CPU renderer, tested on an Intel Core i7-10700K with GCC 13.3 and 16 render threads:

    - **Dragons:** [bonus_bvh_dragons_max_render.yaml](assets/scenes/bonus_bvh_dragons_max_render.yaml), 1000×800, with six dragon meshes and transparent bounding boxes.
    - **Simple:** [simple_scene.yaml](assets/scenes/simple_scene.yaml), 800×400, with one sphere and a checkerboard plane.

    | Build | Compiler flags | Dragons (s) | Simple (ms) |
    | --- | --- | ---: | ---: |
    | Unoptimized Debug | `-O0 -g` | 47.07 | 128.5 |
    | Debugging | `-Og -g` | 8.22 | 27.8 |
    | O2 | `-O2` | 5.04 | 21.8 |
    | Current Debug | `-O2 -g` | 5.05 | 22.6 |
    | O3 | `-O3` | 4.38 | 25.4 |
    | O3 with symbols | `-O3 -g` | 4.42 | 23.4 |
    | Release | `-O3 -DNDEBUG` | 4.39 | 21.6 |
    | RelWithDebInfo | `-O2 -g -DNDEBUG` | 5.10 | 24.7 |


- I tried to use `constexpr` everywhere, but I don't think it helped much with performance; it didn't move any of the render work from runtime to compile time.
- Small improvements to bounding boxes, skipping identity transforms, and so on helped with render times.
- We stand on the shoulders of giants. There are so many algorithms and techniques for
  every minute detail. I am still bothered by aliasing of shapes. Later, Chapter 17
  made it clear that I need to implement anti-aliasing, texture mapping, blurs,
  different lights, and so many other possibilities.

The most rewarding page I found while implementing this was the [teapot competition](https://graphics.cs.utah.edu/courses/cs6620/fall2013/?prj=13); I loved learning that the teapot was first rendered about 50 years ago.

## Other resources

- [Physically Based Rendering](https://pbr-book.org/4ed/contents) — a detailed reference I would like to explore further.
- [Ray Tracing in One Weekend series](https://raytracing.github.io/) — another introduction to ray tracing that I wish I had looked at earlier.

## License

This project is licensed under the [MIT License](LICENSE).
