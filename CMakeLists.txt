cmake_minimum_required(VERSION 3.16)
project(sdl_app C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH /usr/local)

find_package(SDL3 REQUIRED)
find_package(PortAudio REQUIRED)
find_package(PortMidi REQUIRED)

file(GLOB SOURCES 
  "src/*.c"
)
add_executable(app ${SOURCES})
target_link_libraries(app SDL3::SDL3 portaudio portmidi m)
