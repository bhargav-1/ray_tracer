# Ray Tracer Challenge

A CPU ray tracer and scene editor based on *The Ray Tracer Challenge*.

## Geometry examples

- [Building a dodecahedron from triangles](docs/dodecahedron.md): the bowl diagram, vertex coordinates, and face connections.
- [Triangle solids scene](assets/scenes/triangle_solids.yaml): editable octahedron, triangular and square pyramids, decahedron, and dodecahedron.
- [Teapot comparison, Chapter 15](assets/scenes/teapot_comparision_chapter15.yaml): identical teapots with flat and smooth shading.

## OBJ models

Use **+ Obj** in the editor's Add panel to choose an `.obj` file. It becomes one scene object with the usual transform, visibility, material, and shadow controls. Select a group first to import into it. The inspector's **Browse OBJ** button changes the source file.

In YAML, use `type: obj` and `source: ../models/teapot.obj`, alongside the usual `transform`, `material`, and `cast_shadow` fields. Relative sources resolve from the YAML file's folder. Loaded sources are saved as absolute paths so Save As keeps the references valid.

Currently supported: `v` vertices, plain positive `f` indices (including convex polygons), and named `g` groups. Texture coordinates, normals, slash-separated face indices, and OBJ material files are not supported yet.
