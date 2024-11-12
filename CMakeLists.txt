cmake_minimum_required(VERSION 3.30)
project(ed++)
add_executable(
  ed++
  main.cc
  editor.cc
)
find_library(EDIT_LIBRARY NAMES edit)
find_library(CURSES_LIBRARY NAMES curses)
target_link_libraries(ed++ PRIVATE
  ${EDIT_LIBRARY}
  ${CURSES_LIBRARY}
)
target_compile_options(ed++ PRIVATE
  -Wall
  -g
  -DHAVE_PLEDGE
)
