add_library(
  ${PROJECT_NAME} STATIC

  NF_Image.cpp
)

# xcb libary 
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)
pkg_check_modules(XCBIMAGE REQUIRED IMPORTED_TARGET xcb-image)

# image prosessing library
pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)
pkg_check_modules(LIBJPEG REQUIRED IMPORTED_TARGET libjpeg)

# Link library to project
target_include_directories(NF_Image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(
  ${PROJECT_NAME} PUBLIC

  # add xcb library
  PkgConfig::XCB
  PkgConfig::XCBIMAGE

  # add image processiong Library
  PkgConfig::LIBPNG
  PkgConfig::LIBJPEG
)