#include "ShortcutManager.hpp"
#include "WindowManagers.hpp"

#include <stdexcept>
#include <iostream>

#include "Utils/Log.hpp"

using namespace NFWM;

ShortcutManager::ShortcutManager(WindowManagers *wm) : _wm(wm)
{
  if (!wm)
  {
    throw std::runtime_error("EventManager doesn't accept null wm");
  }

  _conn = _wm->GetConnection();
  _screen = _wm->GetScreen();
}

ShortcutManager::~ShortcutManager()
{
}

void ShortcutManager::RegisterShortcut(uint16_t modifiers, xcb_keysym_t keysym, ShortcutHandler_t func, Varians_t variants)
{
  uint32_t combinedKey = (modifiers << 24) | keysym;

  // auto scb = [this, func](WindowManagers *wm, std::variant<int, float, std::string> params)
  // {
  //   std::visit([&](auto &&param)
  //              { func(wm, param); },
  //              params);
  // };

  _shortcuts[combinedKey] = {func, variants};

  xcb_key_symbols_t *keySymbol = xcb_key_symbols_alloc(_conn);
  xcb_keycode_t *keycode;
  keycode = (!(keysym) ? nullptr : xcb_key_symbols_get_keycode(keySymbol, keysym));
  xcb_key_symbols_free(keySymbol);

  if (keycode == XCB_NO_SYMBOL)
  {
    ERROR_LOG("NO SYMBOL")
    free(keycode);
  }

  xcb_void_cookie_t cookie = xcb_grab_key_checked(_conn, 1, _screen->root, modifiers, *keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  xcb_generic_error_t *err = xcb_request_check(_conn, cookie);

  if (keycode)
    free(keycode);

  if (err)
  {
    ERROR_LOG("Error when grab key : ", err->error_code)
    free(err);
  }
}

void ShortcutManager::HandleShortcut(uint16_t modifiers, xcb_keysym_t keysym)
{
  uint32_t combinedKey = (modifiers << 24) | keysym;
  auto it = _shortcuts.find(combinedKey);

  if (it != _shortcuts.end())
  {
    // args->second;

    it->second.func(_wm, it->second.args);
  }
}