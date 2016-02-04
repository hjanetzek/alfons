
## Harfbuzz - ICU-Common - UCDN - Freetype2 ##
##############################################
set(HARFBUZZ_BUILD_ICU ON CACHE BOOL "Enable building of ICU")
set(HARFBUZZ_BUILD_UCDN ON CACHE BOOL "Enable building of UCDN")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz-icu-freetype)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glm)

message(STATUS "harfbuzz" ${HARFBUZZ_LIBRARIES})

set(ALFONS_DEPS_LIBRARIES
  ${ALFONS_DEPS_LIBRARIES}
  harfbuzz ${HARFBUZZ_LIBRARIES}
CACHE INTERNAL "alfons-libs" FORCE)

# set(ALFONS_DEPS_INCLUDE_DIRS
#   ${ALFONS_DEPS_INCLUDE_DIRS}
#   CACHE INTERNAL "alfons-includes" FORCE)

# set(ALFONS_DEPS_LIBRARIES
#   ${ALFONS_DEPS_LIBRARIES}
#   harfbuzz ${HARFBUZZ_LIBRARIES}
#   CACHE INTERNAL "alfons-libs" FORCE)

# set(ALFONS_DEPS_INCLUDE_DIRS
#   ${ALFONS_DEPS_INCLUDE_DIRS}
#   ${PROJECT_SOURCE_DIR}/core/include/glm
#   CACHE INTERNAL "alfons-includes" FORCE)
