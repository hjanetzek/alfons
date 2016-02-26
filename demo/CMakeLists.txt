add_definitions(-DGL_GLEXT_PROTOTYPES)
add_definitions(-DGLM_FORCE_RADIANS)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLFW_INCLUDE_DIRS})

add_executable( basic
  basic.cpp)

target_link_libraries( basic
  LINK_PUBLIC
  alfons
  alfons_render
  # nanovg
  ${GLFW_STATIC_LIBRARIES})

add_executable( enspiraling
  enspiraling.cpp)

target_link_libraries( enspiraling
  LINK_PUBLIC
  alfons
  alfons_render
  ${GLFW_STATIC_LIBRARIES})

# add_executable( wrap
#   wrap.cpp)

# target_link_libraries( wrap
#   LINK_PUBLIC
#   alfons
#   alfons_render
#   alfons_wrap
#   ${GLFW_STATIC_LIBRARIES})

# add_executable (demo_wrap
#   demoWrap.cpp)

# target_include_directories (demo_wrap
#   PUBLIC
#   ${CMAKE_CURRENT_SOURCE_DIR})

# target_link_libraries (demo_wrap
#   LINK_PUBLIC
#   alfons
#   alfons_wrap)

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../fonts/
    ${CMAKE_BINARY_DIR}
  # Make resources show up in IDE, at least this should do it
  SOURCES ${RESOURCE_FILES})

add_dependencies(basic copy_resources)
