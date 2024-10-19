# Function for copying dlls into output folder
function(windeployqt target dir)
    # POST_BUILD step
    # - after build, we have a bin/lib for analyzing qt dependencies
    # - we run windeployqt on target and deploy Qt libs
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
       set(qt_release "--release")
       message("windeployqt build type is Release, using qt release dlls")
    else()
      message("windeployqt build type isn't' Release, using qt debug dlls")
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${QT_PATH}/bin/windeployqt.exe"
                --verbose 1
                ${qt_release}
                --no-translations
                --no-svg
                --no-angle
                --no-opengl
                --no-opengl-sw
                --no-compiler-runtime
                --no-system-d3d-compiler
                --dir "${dir}"
                \"$<TARGET_FILE:${target}>\"
        COMMENT "Deploying Qt libraries using windeployqt for compilation target '${target}' ..."
    )
endfunction()
