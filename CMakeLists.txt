cmake_minimum_required(VERSION 3.20)
project(NeonFrameWM)

include(UsePkgConfig)
find_package(PkgConfig REQUIRED)
if(PkgConfig_FOUND)
  message("PkgConfig is Found")
endif()

message(${CMAKE_BUILD_TYPE})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
message("adding compile devinition")
  add_compile_definitions(_DEBUG=1)
endif()

add_subdirectory(src)