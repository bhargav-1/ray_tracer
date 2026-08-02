# Testing

# Creating Test targets
add_executable(app_lib_tests)

target_sources(app_lib_tests
        PRIVATE
        tuple_point_vector_chapter_1.cpp
        canvas_drawing_chapter2.cpp
        matrices_chapter_3.cpp
        matrix_transformations_chapter4.cpp
        ray_sphere_intersections_chapter5.cpp
        light_and_shading_chapter6.cpp
        making_a_scene_chapter7.cpp
        shadows_chapter8.cpp
        planes_chapter9.cpp
        patterns_chapter10.cpp
        reflection_refraction_chapter11.cpp
        cubes_chapter12.cpp
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
