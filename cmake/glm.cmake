if (NOT DEFINED ${GLM_INCLUDE_DIR})
  message(STATUS "Looking for GLM")

  find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS
      ${GLM_ROOT}
    PATH_SUFFIXES include
    PATHS
      ${GLM_ROOT})


  if(${GLM_INCLUDE_DIR} STREQUAL "GLM_INCLUDE_DIR-NOTFOUND")
    message(STATUS "Checkout GLM")

    include(ExternalProject)

    ExternalProject_Add(glm
      GIT_REPOSITORY https://github.com/g-truc/glm.git
      GIT_TAG 6e5f42b

      # put checkout into build-dir/glm
      PREFIX deps/glm
      SOURCE_DIR glm
      # Use source dir for build dir
      BUILD_IN_SOURCE 1

      UPDATE_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND "")

    set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/glm)

    set(USE_OWN_GLM 1)

  endif()
endif()

message(STATUS "GLM include ${GLM_INCLUDE_DIR}")
