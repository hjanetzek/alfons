

add_library (alfons
  alfons/alfons.cpp
  alfons/fontManager.cpp
  alfons/fontFace.cpp
  alfons/langHelper.cpp
  alfons/font.cpp
  alfons/textBatch.cpp
  alfons/atlas.cpp
  alfons/textShaper.cpp
  alfons/quadMatrix.cpp
  alfons/path/lineSampler.cpp
  alfons/path/splinePath.cpp
  # TODO part of icu-extras
  alfons/scrptrun.cpp)

target_include_directories (alfons
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ALFONS_DEPS_INCLUDE_DIRS})

target_link_libraries (alfons
  LINK_PUBLIC
  PUBLIC
  linebreak
  ${ALFONS_DEPS_LIBRARIES})

### check for undefined symbols
if(BUILD_SHARED_LIBS AND CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries( alfons PRIVATE "-z defs")
endif()

########################################
# Renderer
########################################
if(ALFONS_BUILD_RENDERER)
  add_library (alfons_render
    alfons/renderer/renderer.cpp)

  target_include_directories (alfons_render
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ALFONS_RENDER_DEPS_INCLUDE_DIRS})

  target_link_libraries (alfons_render 
    LINK_PUBLIC
    alfons
    ${ALFONS_RENDER_DEPS_LIBRARIES})

endif()

########################################
# Wrap
########################################

add_library (alfons_wrap
  alfons/lineWrap.cpp)

target_include_directories (alfons_wrap
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries (alfons_wrap
  LINK_PUBLIC
  alfons)

