# Testing

# Creating Test targets
add_executable(app_lib_tests)

target_sources(app_lib_tests
        PRIVATE
        chapter_01_tuples.cpp
        chapter_02_canvas.cpp
        chapter_03_matrices.cpp
        chapter_04_transformations.cpp
        chapter_05_ray_sphere_intersections.cpp
        chapter_06_lighting.cpp
        chapter_07_scene.cpp
        chapter_08_shadows.cpp
        chapter_09_planes.cpp
        chapter_10_patterns.cpp
        chapter_11_reflection_refraction.cpp
        chapter_12_cubes.cpp
        chapter_13_cylinders.cpp
        chapter_14_groups.cpp
        bonus_bounding_boxes.cpp
        chapter_15_triangles.cpp
        chapter_16_csg.cpp
        shape_catalog.cpp
        scene_model.cpp
        scene_yaml_io.cpp
        scene_compiler.cpp
        scene_editor_app.cpp
        scene_renderer.cpp
)

target_link_libraries(app_lib_tests
        PRIVATE
        app_lib
        Catch2::Catch2WithMain
)

# Now we can include testing stuff to discover tests
include(Catch)
#set(CMAKE_CATCH_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
catch_discover_tests(app_lib_tests)
