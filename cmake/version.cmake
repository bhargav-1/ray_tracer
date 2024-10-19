if (DEFINED ENV{GIT_REV})
  set(GIT_REV $ENV{GIT_REV})
  message("Git Rev ENV set: GIT_REV: ${GIT_REV}")
else ()
  execute_process(COMMAND git log --pretty=format:'%h' -n 1
    OUTPUT_VARIABLE GIT_REV
    ERROR_QUIET)
endif ()

message("------------------------------GIT_REV: ${GIT_REV}")
# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
  set(GIT_REV "N/A")
  set(GIT_DIFF "")
  set(GIT_TAG "vN/A")
  set(GIT_BRANCH "N/A")
else ()

  if (DEFINED ENV{GIT_DIFF})
    set(GIT_DIFF $ENV{GIT_DIFF})
    message("Git Diff ENV set: GIT_DIFF: ${GIT_DIFF}")
  else ()
    execute_process(
      COMMAND bash -c "git diff --quiet --exit-code || echo +"
      OUTPUT_VARIABLE GIT_DIFF)
  endif ()

  if (DEFINED ENV{GIT_TAG})
    set(GIT_TAG $ENV{GIT_TAG})
    message("Git Tag ENV set: GIT_TAG: ${GIT_TAG}")
  else ()
    execute_process(
      COMMAND git describe --tags --abbrev=0
      OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
  endif ()

  if (DEFINED ENV{GIT_BRANCH})
    set(GIT_BRANCH $ENV{GIT_BRANCH})
    message("Git Branch ENV set: GIT_BRANCH: ${GIT_BRANCH}")
  else ()
    execute_process(
      COMMAND git rev-parse --abbrev-ref HEAD
      OUTPUT_VARIABLE GIT_BRANCH)
  endif ()

  string(STRIP "${GIT_REV}" GIT_REV)
  string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
  string(STRIP "${GIT_DIFF}" GIT_DIFF)
  string(STRIP "${GIT_TAG}" GIT_TAG)
  string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif ()

# Splits tag for use by project VERSION
set(GIT_TAG_SPLIT "${GIT_TAG}")
string(REPLACE "v" " " GIT_TAG_SPLIT "${GIT_TAG_SPLIT}")
string(STRIP "${GIT_TAG_SPLIT}" GIT_TAG_SPLIT)

message("------------------------------GIT_REV: ${GIT_REV}")
message("------------------------------GIT_DIFF: ${GIT_DIFF}")
message("------------------------------GIT_TAG: ${GIT_TAG}")
message("------------------------------GIT_BRANCH: ${GIT_BRANCH}")

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
function(version_generate)
  set(VERSION "#pragma once
#define GIT_REV ${GIT_REV}${GIT_DIFF} // NOLINT
#define GIT_TAG ${GIT_TAG}  // NOLINT
#define GIT_BRANCH ${GIT_BRANCH} // NOLINT

#define RELEASE_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR}
#define RELEASE_MINOR ${CMAKE_PROJECT_VERSION_MINOR}
#define RELEASE_PATCH ${CMAKE_PROJECT_VERSION_PATCH}
#define RELEASE_VERSION_STRING \"v${CMAKE_PROJECT_VERSION}\"

#define XSTR(x) STR(x)
#define STR(x) #x
#define BUILD_VERSION_STRING XSTR(GIT_TAG) \":\" XSTR(GIT_BRANCH) \"-\" XSTR(GIT_REV)
#define DEVICE_NAME XSTR(_BUILD_TARGET_NAME)")

  if (EXISTS ${GENERATED_BUILD_DIR}/version.h )
    file(READ ${GENERATED_BUILD_DIR}/version.h  VERSION_)
  else ()
    set(VERSION_ "")
  endif ()

  if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${GENERATED_BUILD_DIR}/version.h "${VERSION}")
  endif ()
  add_custom_command(
    OUTPUT ${GENERATED_BUILD_DIR}/version.h
    ${GENERATED_BUILD_DIR}/version.h
    COMMAND ${CMAKE_COMMAND} -P
    ${PROJECT_SOURCE_DIR}/version.cmake)

  set_property(GLOBAL PROPERTY ADDITIONAL_CLEAN_FILES ${GENERATED_BUILD_DIR}/version.h)
endfunction()
