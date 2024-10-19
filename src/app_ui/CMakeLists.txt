
# Declare library that will hold all code except main.cpp
# useful for linking to main + test without a lot of redefinition
add_library(${PROJECT_NAME}_lib)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
    src/add.cpp
)
#set_target_properties(${PROJECT_NAME}_lib PROPERTIES INCLUDE_DIRECTORIES ${GENERATED_BUILD_DIR}/src)
target_include_directories(${PROJECT_NAME}_lib
  PUBLIC
  inc
  ${GENERATED_BUILD_DIR}
)
target_link_libraries(${PROJECT_NAME}_lib
  PUBLIC
  fmt::fmt
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  simpleble::simpleble
  simpleble::simpleble-c
  app_lib
)

add_subdirectory(test)

# Declare actual executable
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE
        src/main.cpp
)

# Qt will show a console in background on windows unless WIN32_EXECUTABLE is true
# Only do this on Release builds so we can see the log during debug
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET main PROPERTY WIN32_EXECUTABLE true)
endif()

# Add in CMake Generated files (version.h etc)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${PROJECT_NAME}_lib
)

install(TARGETS ${PROJECT_NAME}
  EXPORT "${PROJECT_NAME}Targets"
  COMPONENT ${PROJECT_NAME} # must be here, not any line lower
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # include
)

if (WIN32)
# Want to add Qt .dlls to build folder; This has been replace by using $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
# But it was a pain in the ass to find and I might rewrite history, so keeping until we're sure we don't need it
#  include(windeployqt)
#  windeployqt(${PROJECT_NAME} "${PROJECT_BINARY_DIR}/qtDeploy/")
#  windeployqt(${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/qtDeploy/")
  install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
      COMPONENT ${PROJECT_NAME}
      TYPE BIN
  )
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )
  endif()
