
add_library(app_lib)

target_sources(app_lib
        PRIVATE
        src/scene_editor/gl_texture.cpp
        src/scene_editor/add_panel.cpp
        src/scene_editor/camera_controls.cpp
        src/scene_editor/widget_helpers.cpp
        src/scene_editor/object_panel.cpp
        src/scene_editor/render_controller.cpp
        src/scene_editor/scene_debug_log.cpp
        src/scene_editor/scene_panel.cpp
        src/scene_editor/scene_renderer.cpp
        src/scene_editor/scene_editor_app.cpp
        src/scene_editor/viewport_overlay.cpp
        src/scene_editor/viewport_panel.cpp
        src/scene/scene_yaml.cpp
        src/scene/scene_compiler.cpp
)
#set_target_properties(${PROJECT_NAME}_lib PROPERTIES INCLUDE_DIRECTORIES ${GENERATED_BUILD_DIR}/src)
target_include_directories(app_lib
        PUBLIC
        inc
        ${GENERATED_BUILD_DIR}
)
target_link_libraries(app_lib
        PUBLIC
        fmt::fmt
        yaml-cpp::yaml-cpp
)
target_compile_definitions(app_lib
        PUBLIC
        RAY_TRACER_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/inc/app/scene" PREFIX "Header Files\\scene"
        FILES inc/app/scene/scene.h inc/app/scene/scene_yaml.h inc/app/scene/scene_compiler.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/inc/app/scene_editor" PREFIX "Header Files\\scene_editor"
        FILES
        inc/app/scene_editor/add_panel.h
        inc/app/scene_editor/camera_controls.h
        inc/app/scene_editor/gl_texture.h
        inc/app/scene_editor/imgui_file_dialog.h
        inc/app/scene_editor/widget_helpers.h
        inc/app/scene_editor/object_panel.h
        inc/app/scene_editor/render_controller.h
        inc/app/scene_editor/scene_debug_log.h
        inc/app/scene_editor/scene_editor_app.h
        inc/app/scene_editor/scene_panel.h
        inc/app/scene_editor/scene_renderer.h
        inc/app/scene_editor/render_state.h
        inc/app/scene_editor/viewport_overlay.h
        inc/app/scene_editor/viewport_panel.h)

add_executable(projectile_canvas)
target_sources(projectile_canvas
        PRIVATE
        src/main.cpp
)
target_link_libraries(projectile_canvas
        PRIVATE
        app_lib
)
add_executable(clock_canvas)
target_sources(clock_canvas
        PRIVATE
        src/clock_canvas.cpp
)
target_link_libraries(clock_canvas
        PRIVATE
        app_lib
)

add_library(imgui_lib)
target_sources(imgui_lib
        PRIVATE)
target_include_directories(imgui_lib
        PUBLIC
        ${CMAKE_SOURCE_DIR}/
)
target_link_libraries(imgui_lib
        PUBLIC
        glfw
        OpenGL
        imgui::imgui
)

add_executable(imgui_example)
target_sources(imgui_example
        PRIVATE
        src/imgui_test.cpp
)
set_source_files_properties(src/imgui_test.cpp PROPERTIES
        SKIP_LINTING ON
)
target_link_libraries(imgui_example
        PRIVATE
        imgui_lib
)

add_executable(imgui_clock)
target_sources(imgui_clock
        PRIVATE
        src/imgui_clock.cpp
)
target_link_libraries(imgui_clock
        PRIVATE
        app_lib
        imgui_lib
)

add_executable(ray_sphere_casting_chapter5)
target_sources(ray_sphere_casting_chapter5
        PRIVATE
        src/ray_sphere_casting_chapter5_app.cpp
)
set_source_files_properties(src/ray_sphere_casting_chapter5_app.cpp PROPERTIES
        SKIP_LINTING ON
)

target_link_libraries(ray_sphere_casting_chapter5
        PRIVATE
        app_lib
        imgui_lib
)

add_executable(light_and_shading_chapter6)
target_sources(light_and_shading_chapter6
        PRIVATE
        src/light_and_shading_chapter6_app.cpp
)
set_source_files_properties(src/light_and_shading_chapter6_app.cpp PROPERTIES
        SKIP_LINTING ON
)

target_link_libraries(light_and_shading_chapter6
        PRIVATE
        app_lib
        imgui_lib
)

add_executable(making_a_scene_chapter7)
target_sources(making_a_scene_chapter7
        PRIVATE
        src/world_canvas_chapter7.cpp
)
set_source_files_properties(src/world_canvas_chapter7.cpp PROPERTIES
        SKIP_LINTING ON
)

target_link_libraries(making_a_scene_chapter7
        PRIVATE
        app_lib
        imgui_lib
)
add_executable(making_a_scene_chapter7_imgui)
target_sources(making_a_scene_chapter7_imgui
        PRIVATE
        src/world_imgui_chapter7.cpp
)
set_source_files_properties(src/world_imgui_chapter7.cpp inc/app/canvas.h PROPERTIES
        SKIP_LINTING ON
)

target_link_libraries(making_a_scene_chapter7_imgui
        PRIVATE
        app_lib
        imgui_lib
)
add_executable(eclipse_chapter8)
target_sources(eclipse_chapter8
        PRIVATE
        src/shadows_eclipse_chapter_8.cpp
)
set_source_files_properties(src/shadows_eclipse_chapter_8.cpp PROPERTIES
        SKIP_LINTING ON
)

target_link_libraries(eclipse_chapter8
        PRIVATE
        app_lib
        imgui_lib
)
add_executable(plane_chapter9)
target_sources(plane_chapter9
        PRIVATE
        src/plane_imgui_chapter9.cpp
)
set_source_files_properties(src/plane_imgui_chapter9.cpp PROPERTIES
        SKIP_LINTING ON
)
target_link_libraries(plane_chapter9
        PRIVATE
        app_lib
        imgui_lib
)
add_executable(patterns_chapter10)
target_sources(patterns_chapter10
        PRIVATE
        src/chapter_10_patterns.cpp
)
set_source_files_properties(src/chapter_10_patterns.cpp PROPERTIES
        SKIP_LINTING ON
)
target_link_libraries(patterns_chapter10
        PRIVATE
        app_lib
        imgui_lib
)

add_executable(patterns_chapter10_perturbed)
target_sources(patterns_chapter10_perturbed
        PRIVATE
        src/chapter_10_patterns_perturbed.cpp
)
set_source_files_properties(src/chapter_10_patterns_perturbed.cpp PROPERTIES
        SKIP_LINTING ON
)
target_link_libraries(patterns_chapter10_perturbed
        PRIVATE
        app_lib
        imgui_lib
)
add_executable(scene_editor)
target_sources(scene_editor
        PRIVATE
        src/scene_editor/imgui_file_dialog.cpp
        src/scene_editor_main.cpp
)
target_link_libraries(scene_editor
        PRIVATE
        app_lib
        imgui_lib
)
add_subdirectory(test)

install(TARGETS app_lib
        EXPORT "${PROJECT_NAME}Targets"
        COMPONENT ${PROJECT_NAME} # must be here, not any line lower
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/app
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/app # include
)
