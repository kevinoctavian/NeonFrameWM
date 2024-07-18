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

add_subdirectory(NeonFrame/NeonFrameWM)
add_subdirectory(NeonFrame/NF_Image)

# installing project to /usr/bin
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin)

# installing the session file to /usr/share/xsession
install(FILES NeonFrameWM.desktop DESTINATION /usr/share/xsessions)

#installing default NeonFrameWM config toml file
install(SCRIPT cmake/post_install.cmake)
install(FILES NeonFrameConfig.toml DESTINATION /usr/share/neonframewm)