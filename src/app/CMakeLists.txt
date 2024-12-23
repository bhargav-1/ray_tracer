
add_library(app_lib)

target_sources(app_lib
        PRIVATE
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
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)

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

add_subdirectory(test)

install(TARGETS app_lib
        EXPORT "${PROJECT_NAME}Targets"
        COMPONENT ${PROJECT_NAME} # must be here, not any line lower
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/app
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/app # include
)
