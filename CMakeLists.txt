cmake_minimum_required(VERSION 3.16)
project(sdl_app C)

find_package(SDL3 REQUIRED)

file(GLOB SOURCES 
  "src/*.c"
)
add_executable(app ${SOURCES})
target_link_libraries(app SDL3::SDL3)
