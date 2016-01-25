add_library (linebreak
  linebreak/linebreak.c
  linebreak/linebreakdata.c
  linebreak/linebreakdef.c)

target_include_directories (linebreak
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/linebreak)
