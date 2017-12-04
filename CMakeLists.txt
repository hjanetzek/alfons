cmake_minimum_required(VERSION 2.8.12)

if (APPLE)
  option(HB_HAVE_CORETEXT "Enable CoreText shaper backend on macOS" ON)
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/glm.cmake)

if (NOT DEFINED ALFONS_DEPS_LIBRARIES)

  include(FindPkgConfig)
  pkg_check_modules(ICU REQUIRED icu-uc)
  pkg_check_modules(FREETYPE REQUIRED freetype2)
  if (APPLE)
      pkg_check_modules(HARFBUZZ REQUIRED harfbuzz>=1.7.2)
  else()
      pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)
  endif()

  set(ALFONS_DEPS_LIBRARIES
    ${ICU_LDFLAGS}
    ${FREETYPE_LDFLAGS}
    ${HARFBUZZ_LDFLAGS}
    CACHE INTERNAL "alfons-libs" FORCE)

  set(ALFONS_DEPS_INCLUDE_DIRS
    ${ICU_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${HARFBUZZ_INCLUDE_DIRS}
    CACHE INTERNAL "alfons-includes" FORCE)

endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
