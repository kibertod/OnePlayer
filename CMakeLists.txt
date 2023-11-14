cmake_minimum_required(VERSION 3.27)
project(oneplayer VERSION 0.1.0 LANGUAGES CXX)
set(SOURCES
    src/main.cpp
    src/Ueberzug.cpp
    src/Variable.cpp
    src/Widget.cpp)
add_executable(oneplayer ${SOURCES})
target_include_directories(oneplayer PRIVATE src/includes)
set_target_properties(oneplayer PROPERTIES CXX_STANDARD 20)
target_compile_options(oneplayer PRIVATE 
    -Wall
    -Wextra
    -Wpedantic
    -Werror)
include(FindCurses)
target_link_libraries(oneplayer ${CURSES_LIBRARIES})
target_include_directories(oneplayer PRIVATE ${CURSES_INCLUDE_DIRS})

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 
target_include_directories(oneplayer PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(oneplayer ${CURL_LIBRARIES})
