#pragma once

#include <xcb/xcb.h>
#include <unordered_map>
#include <functional>
#include <string>

#include "Constants/keysym.hpp"
#include "ShortcutHandler.hpp"

namespace NFWM
{
  class WindowManagers;

  using ShortcutKey_t = std::function<void(WindowManagers *, Varians_t)>;

  class ShortcutManager
  {
  private:
    struct Shortcuts_t
    {
      ShortcutKey_t func;
      Varians_t args;
    };

    WindowManagers *_wm;

    xcb_connection_t *_conn;
    xcb_screen_t *_screen;

    std::unordered_map<xcb_keysym_t, Shortcuts_t> _shortcuts;

  public:
    ShortcutManager(WindowManagers *wm);
    ~ShortcutManager();

    void RegisterShortcut(uint16_t modifier, xcb_keysym_t keysym, ShortcutHandler_t func, Varians_t variants);

    void HandleShortcut(uint16_t modifier, xcb_keysym_t keysym);
  };

} // namespace NFWM
