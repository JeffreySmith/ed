cmake_minimum_required(VERSION 3.30)
set (CMAKE_CXX_STANDARD 23)
project(ed++)
add_executable(
  ed++
  main.cc
  editor.cc
  shell.cc
)
find_library(EDIT_LIBRARY NAMES edit)
find_library(CURSES_LIBRARY NAMES curses)
target_link_libraries(ed++ PRIVATE
  ${EDIT_LIBRARY}
  ${CURSES_LIBRARY}
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g -DHAVE_PLEDGE")
