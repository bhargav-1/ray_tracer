# Web-only dependencies: host Ubuntu libraries cannot be linked into WebAssembly.
include(FetchContent)
# yaml-cpp 0.8 declares an older minimum; CMake 4 needs this policy floor.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_SHARED_LIBS OFF)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)

# Keep exception handling inside Wasm instead of routing calls through JavaScript.
add_compile_options(-pthread -fwasm-exceptions)
# Keep growable memory for large scenes while using rendering threads.
add_link_options(-Wno-pthreads-mem-growth)
add_link_options(-pthread -fwasm-exceptions -sUSE_GLFW=3
    -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
    -sMALLOC=mimalloc
    # Start at 256 MiB to give the threaded allocator more room.
    -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=268435456
    "-sPTHREAD_POOL_SIZE=Math.max(1,(navigator.hardwareConcurrency||1)-2)"
    -sPTHREAD_POOL_SIZE_STRICT=2)

FetchContent_Declare(yaml-cpp
    SYSTEM
    URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(yaml-cpp)
target_compile_options(yaml-cpp PRIVATE -w)

FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(imgui)
add_library(imgui_web STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui_web SYSTEM PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui_web PUBLIC IMGUI_IMPL_OPENGL_ES3)
target_compile_options(imgui_web PRIVATE -w)
add_library(imgui::imgui ALIAS imgui_web)
