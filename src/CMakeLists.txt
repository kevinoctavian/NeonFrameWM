add_executable(
  ${PROJECT_NAME}

  # Main File
  main.cpp
  
  # Client
  ${PROJECT_NAME}/Client/Client.cpp
  ${PROJECT_NAME}/Client/ClientManager.cpp

  # Config

  # Utils
  ${PROJECT_NAME}/Utils/ColorManager.cpp
  ${PROJECT_NAME}/Utils/Cursor.cpp

  # Window

  # Main Window Manager
  ${PROJECT_NAME}/WindowManagers.cpp
  ${PROJECT_NAME}/EventManager.cpp
  ${PROJECT_NAME}/ShortcutManager.cpp
  ${PROJECT_NAME}/ShortcutHandler.cpp
  
)

# Check Modules Library
# xcb and xcb extention modules
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)
pkg_check_modules(XCBEWMH REQUIRED IMPORTED_TARGET xcb-ewmh)
pkg_check_modules(XCBICCCM REQUIRED IMPORTED_TARGET xcb-icccm)
pkg_check_modules(XCBRENDER REQUIRED IMPORTED_TARGET xcb-render)
pkg_check_modules(XCBKEYSYMS REQUIRED IMPORTED_TARGET xcb-keysyms)
pkg_check_modules(XCBCURSOR REQUIRED IMPORTED_TARGET xcb-cursor)
pkg_check_modules(XCBEVENT REQUIRED IMPORTED_TARGET xcb-event)
pkg_check_modules(XCBIMAGE REQUIRED IMPORTED_TARGET xcb-image)

# more modules
# wallpaper utility
pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)
pkg_check_modules(LIbJPEG REQUIRED IMPORTED_TARGET libjpeg)

# Link library to project
target_link_libraries(
  ${PROJECT_NAME} PRIVATE

  # XCB LIBRARY
  PkgConfig::XCB
  PkgConfig::XCBEWMH
  PkgConfig::XCBICCCM
  PkgConfig::XCBRENDER
  PkgConfig::XCBKEYSYMS
  PkgConfig::XCBCURSOR
  PkgConfig::XCBEVENT
  PkgConfig::XCBIMAGE

  # WALPAPER (JPG, PNG) Library
  PkgConfig::LIBPNG
  PkgConfig::LIbJPEG
)
