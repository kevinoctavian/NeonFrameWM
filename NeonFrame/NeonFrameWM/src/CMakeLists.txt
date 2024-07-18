add_executable(
  ${PROJECT_NAME}

  # Main File
  main.cpp
  
  # Client
  Client/Client.cpp
  Client/ClientManager.cpp
  Client/DesktopManager.cpp

  # Config

  # Utils
  Utils/ColorManager.cpp
  Utils/Cursor.cpp

  # Window

  # Main Window Manager
  WindowManagers.cpp
  EventManager.cpp
  ShortcutManager.cpp
  ShortcutHandler.cpp
  
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
pkg_check_modules(XCBSHAPE REQUIRED IMPORTED_TARGET xcb-shape)
pkg_check_modules(XCBCOMPOSITE REQUIRED IMPORTED_TARGET xcb-composite)

# Link library to project
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../NF_Image/src)
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
  PkgConfig::XCBSHAPE
  PkgConfig::XCBRENDER
  PkgConfig::XCBCOMPOSITE
  
  # Neonframe Image processing
  NF_Image
)
