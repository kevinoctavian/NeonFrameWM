#pragma once

// #include <vector>
#include <memory>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#include "EventManager.hpp"
#include "ShortcutManager.hpp"
#include "Client/DesktopManager.hpp"

#include "NF_Image.hpp"

namespace NFWM
{
  class WindowManagers
  {
  private:
    // connection
    xcb_connection_t *_connection;
    int _screenp;

    // screen
    xcb_screen_t *_screen;
    // Multiple screen/monitor (incoming)
    // std::vector<xcb_screen_t *> _screens;

    // wmcheck
    xcb_window_t _wmCheck;

    // background image/color window
    xcb_window_t _windowBackground;

    // keep running?
    bool isRunning;

    // EWMH
    xcb_ewmh_connection_t *_ewmh;
    xcb_atom_t _WM_DELETE_WINDOW;
    xcb_atom_t _WM_TAKE_FOCUS;

    // Another Manager
    std::unique_ptr<EventManager> _eventManager;
    std::unique_ptr<ShortcutManager> _shortcutManager;
    std::unique_ptr<Desktop::DesktopManager> _desktopManager;

    // background image property
    std::unique_ptr<NF_Image::BackgroundManager> _backgroundManager;

  public:
    WindowManagers(const char *displayName);
    ~WindowManagers();

    int Start();
    bool CheckAnotherWM();
    int Init();
    int InitAtom();
    void GrabKey();
    void GrabButtons(Client::Client *client);

    void Focus(Client::Client *client);
    void UnFocus(Client::Client *client);

    void Quit();

    bool SendEvent(xcb_window_t win, xcb_atom_t proto, int mask, long d0, long d1, long d2, long d3, long d4);

    void RedrawBackground();

    // getter
    xcb_connection_t *GetConnection() { return _connection; }
    xcb_screen_t *GetScreen() { return _screen; }
    xcb_ewmh_connection_t *GetEWMH() { return _ewmh; }
    ShortcutManager *GetShortcutManager() { return _shortcutManager.get(); }
    Desktop::DesktopManager *GetDesktopManager() { return _desktopManager.get(); }
    xcb_atom_t GetDeleteWindowAtom() { return _WM_DELETE_WINDOW; }
    xcb_atom_t GetTakeFocusAtom() { return _WM_TAKE_FOCUS; }
  };
}