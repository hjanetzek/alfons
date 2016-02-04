find_path(GLM_INCLUDE_DIR glm/glm.hpp
  HINTS
    ${GLM_ROOT}
  PATH_SUFFIXES include
  PATHS
    ${GLM_ROOT})


if(${GLM_INCLUDE_DIR} STREQUAL "GLM_INCLUDE_DIR-NOTFOUND")

  include(ExternalProject)

  ExternalProject_Add(glm
    # get 0.9.7
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG eb4a198

    # put checkout into build-dir/glm
    PREFIX ${PROJECT_SOURCE_DIR}/deps/glm
    SOURCE_DIR "glm"
     # Use source dir for build dir
    BUILD_IN_SOURCE 1

    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

  set(GLM_INCLUDE_DIR ${PROJECT_BINARY_DIR}/glm)

  set(USE_OWN_GLM 1)

endif()

message(STATUS "GLM include ${GLM_INCLUDE_DIR}")
