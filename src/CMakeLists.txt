if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

add_library (linebreak
  linebreak/linebreak.c
  linebreak/linebreakdata.c
  linebreak/linebreakdef.c)

target_include_directories (linebreak
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/linebreak)


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
  ${ALFONS_DEPS_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/logger)

target_link_libraries (alfons
  LINK_PUBLIC
  linebreak
  ${ALFONS_DEPS_LIBRARIES})

if (USE_OWN_GLM)
add_dependencies(alfons glm)
endif()

### check for undefined symbols
if(BUILD_SHARED_LIBS AND CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries( alfons PRIVATE "-z defs")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(alfons
    PRIVATE
    -D_USE_MATH_DEFINES
    -utf-8
    -wd4267
    -wd4244)
endif()

