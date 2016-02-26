cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
# include cmake/utils.cmake
include(utils)

if (BUILD_DEMO)
  set(ALFONS_BUILD_RENDERER 1)
  message(STATUS "modules : ${CMAKE_MODULE_PATH}")

  # cmake output configuration
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  # platform lookup
  #set(SUPPORTED_TARGETS darwin ios android raspberrypi linux)
  set(SUPPORTED_TARGETS android linux)

  if(NOT PLATFORM_TARGET)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" varplatform)
    string(TOUPPER "${CMAKE_SYSTEM_NAME}" VARPLATFORM)

    message(STATUS "No target defined (give as parameter -DPLATFORM_TARGET=platform_name).")
    message(STATUS "Targets available: ${SUPPORTED_TARGETS}.")
    message(STATUS "Will use your OS: ${varplatform}.")

    set(PLATFORM_TARGET ${varplatform})
  else()
    string(TOLOWER "${PLATFORM_TARGET}" varplatform)
    string(TOUPPER "${PLATFORM_TARGET}" VARPLATFORM)

    message(STATUS "Platform chosen : ${VARPLATFORM}")
  endif()

  list(FIND SUPPORTED_TARGETS ${varplatform} target_in_list)

  if(target_in_list EQUAL -1)
    message(SEND_ERROR "${varplatform} not in supported targets: ${SUPPORTED_TARGETS}")
    return()
  endif()

  # setting up cache variable platform
  set(PLATFORM ${varplatform} CACHE INTERNAL "platform" FORCE)

  include(glm)

  include(${PLATFORM_TARGET})

  #add_library(nanovg
  #    nanovg/src/nanovg.c)

  #target_include_directories(nanovg
  #    PUBLIC
  #    nanovg/src)
else()
  include(glm)

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps)

  #set(ALFONS_DEPS_INCLUDE_DIRS ${ALFONS_DEPS_INCLUDE_DIRS}
  #  ${CMAKE_CURRENT_SOURCE_DIR}/alfons/src/logger
  #  CACHE INTERNAL "alfons-includes" FORCE)

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()
