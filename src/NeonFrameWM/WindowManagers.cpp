#include "WindowManagers.hpp"

#include <stdexcept>
#include <signal.h>
#include <iostream>
#include <cstring>

#include <xcb/xcb_event.h>
#include <xcb/xcb_icccm.h>

#include "Utils/ColorManager.hpp"
#include "Utils/Cursor.hpp"
#include "Utils/Log.hpp"
#include "Constants/mask.hpp"
#include "Config/KeyboardShortcut.hpp"

namespace NFWM
{
  WindowManagers::WindowManagers(const char *displayName)
  {
    _connection = xcb_connect(displayName, &_screenp);
    int error = xcb_connection_has_error(_connection);

    if (error != 0)
    {
      throw std::runtime_error("Error when trying connect to display server");
    }

    _ewmh = (xcb_ewmh_connection_t *)malloc(sizeof(xcb_ewmh_connection_t));

    if (!_ewmh)
    {
      throw std::runtime_error("Error when trying malloc ewmh to memory");
    }

    {
      // TODO: Create multi onitor screen
      xcb_screen_iterator_t screens = xcb_setup_roots_iterator(xcb_get_setup(_connection));
      _screen = screens.data;
    }

    _eventManager = std::unique_ptr<EventManager>(new EventManager(this));
    _shortcutManager = std::unique_ptr<ShortcutManager>(new ShortcutManager(this));
    _clientManager = std::unique_ptr<Client::ClientManager>(new Client::ClientManager(this));
  }

  WindowManagers::~WindowManagers()
  {
    if (_ewmh)
    {
      xcb_ewmh_connection_wipe(_ewmh);
      free(_ewmh);
    }

    xcb_ungrab_keyboard(_connection, XCB_CURRENT_TIME);
    xcb_disconnect(_connection);
  }

  int WindowManagers::Start()
  {

    if (CheckAnotherWM())
    {
      throw std::runtime_error("there is another WM running! Kill it first before run NFWM");
    }

    Init();

    xcb_flush(_connection);
    xcb_generic_event_t *ev;
    while (isRunning && (ev = xcb_wait_for_event(_connection)))
    {
      _eventManager->RecieveEvents(ev);
      free(ev);
    }

    return 0;
  }

  int WindowManagers::Init()
  {
    if (InitAtom() < 0)
      return -1;

    uint32_t mask = XCB_CW_BACK_PIXEL;
    uint32_t values[1] = {
        Utils::ColorManager::RGBA(184, 0, 211, 1)};

    xcb_void_cookie_t cwCookie = xcb_change_window_attributes_checked(_connection, _screen->root, mask, values);
    xcb_generic_error_t *err = xcb_request_check(_connection, cwCookie);
    if (err)
    {
      std::cout << "Error while change window attributes" << std::endl;
      return -1;
    }

    // Utils::Cursor::create_cursor(_connection, _screen, _screen->root, NFC_watch);
    Utils::Cursor::create_cursor(_connection, _screen, _screen->root, "left_ptr");

    xcb_clear_area(_connection, 0, _screen->root, 0, 0, _screen->width_in_pixels, _screen->height_in_pixels);
    xcb_flush(_connection);

    signal(SIGCHLD, SIG_IGN);

    isRunning = true;

    std::cout << "initialing " << LENGTH(NFWM_SHORTCUT_LIST) << " Shortcut" << std::endl;

    GrabKey();
    GrabButtons(nullptr);
    return 0;
  }

  int WindowManagers::InitAtom()
  {
    xcb_intern_atom_cookie_t *atomsCookie = xcb_ewmh_init_atoms(_connection, _ewmh);
    if (!xcb_ewmh_init_atoms_replies(_ewmh, atomsCookie, nullptr))
    {
      std::cout << "Failed to init the ewmh atoms" << std::endl;
      return -1;
    }

    // set EWMH Atom to root
    {
      const char *atomName = "WM_DELETE_WINDOW";
      xcb_intern_atom_cookie_t wmDeleteC = xcb_intern_atom(_connection, 0, strlen(atomName), atomName);

      atomName = "WM_TAKE_FOCUS";
      xcb_intern_atom_cookie_t wmTakeFocusC = xcb_intern_atom(_connection, 0, strlen(atomName), atomName);

      xcb_intern_atom_reply_t *deleteReply = xcb_intern_atom_reply(_connection, wmDeleteC, nullptr);
      xcb_intern_atom_reply_t *takeFocusReply = xcb_intern_atom_reply(_connection, wmTakeFocusC, nullptr);

      if (!deleteReply)
        return -1;
      _WM_DELETE_WINDOW = deleteReply->atom;

      if (!takeFocusReply)
        return -1;
      _WM_TAKE_FOCUS = takeFocusReply->atom;

      free(deleteReply);
      free(takeFocusReply);

      xcb_atom_t suported_atom[] = {
          _ewmh->_NET_SUPPORTED,
          _ewmh->_NET_SUPPORTING_WM_CHECK,
          _ewmh->_NET_WM_NAME,
          _ewmh->_NET_WM_ICON,
          _ewmh->_NET_WM_STATE,
          _ewmh->_NET_WM_STATE_FULLSCREEN,
          _ewmh->_NET_WM_WINDOW_TYPE,
          _ewmh->_NET_WM_WINDOW_TYPE_DIALOG,
          _ewmh->_NET_ACTIVE_WINDOW,
          _ewmh->_NET_CLIENT_LIST,
          _ewmh->_NET_DESKTOP_NAMES,
          _ewmh->_NET_DESKTOP_VIEWPORT,
          _ewmh->_NET_NUMBER_OF_DESKTOPS,
          _ewmh->_NET_CURRENT_DESKTOP,
      };

      xcb_ewmh_set_supported(_ewmh, 0, LENGTH(suported_atom), suported_atom);
    }

    const char *name = "NeonFrameWM";
    xcb_ewmh_set_wm_name(_ewmh, _screen->root, strlen(name), name);

    int desktop = 0;
    xcb_ewmh_set_current_desktop(_ewmh, 0, desktop);
    xcb_flush(_connection);

    xcb_window_t wmCheck = xcb_generate_id(_connection);
    xcb_create_window(
        _connection,
        XCB_COPY_FROM_PARENT,
        wmCheck,
        _screen->root,
        0, 0, 1, 1, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        _screen->root_visual,
        XCB_EVENT_MASK_NO_EVENT, nullptr);

    xcb_ewmh_set_supporting_wm_check(_ewmh, _screen->root, wmCheck);
    xcb_ewmh_set_wm_name(_ewmh, wmCheck, strlen(name), name);

    xcb_flush(_connection);
    return 0;
  }

  bool WindowManagers::CheckAnotherWM()
  {
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[1] = {WINDOW_MANAGER_STRUCTURE_MASK};

    xcb_void_cookie_t cwCookie = xcb_change_window_attributes_checked(_connection, _screen->root, mask, &values);
    xcb_generic_error_t *err = xcb_request_check(_connection, cwCookie);

    return err != nullptr;
  }

  void WindowManagers::GrabKey()
  {
    xcb_ungrab_key(_connection, XCB_GRAB_ANY, _screen->root, XCB_MOD_MASK_ANY);

    // xcb_grab_keyboard(_connection, 1, _screen->root, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    // setup shortcut
    for (int i = 0; i < LENGTH(NFWM_SHORTCUT_LIST); i++)
    {
      KeyboardShortcut_t ks = NFWM_SHORTCUT_LIST[i];
      _shortcutManager->RegisterShortcut(ks.modifiers, ks.keysym, ks.func, ks.args);
    }

    xcb_flush(_connection);
  }

  void WindowManagers::GrabButtons(Client::Client *client)
  {
    xcb_window_t win;
    if (client)
      win = client->GetWindow();
    else
      win = _screen->root;

    xcb_ungrab_button(_connection, XCB_BUTTON_INDEX_ANY, win, XCB_MOD_MASK_ANY);

    // super key + left click
    xcb_grab_button(_connection, 1, _screen->root,
                    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _screen->root,
                    XCB_NONE, XCB_BUTTON_INDEX_1, NFK_SUPER);

    // super key + middle click
    xcb_grab_button(_connection, 1, _screen->root,
                    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _screen->root,
                    XCB_NONE, XCB_BUTTON_INDEX_2, NFK_SUPER);
    // super key + right click
    xcb_grab_button(_connection, 1, _screen->root,
                    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _screen->root,
                    XCB_NONE, XCB_BUTTON_INDEX_3, NFK_SUPER);
    xcb_grab_button(_connection, 1, _screen->root,
                    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _screen->root,
                    XCB_NONE, XCB_BUTTON_INDEX_4, XCB_NONE);
    xcb_grab_button(_connection, 1, _screen->root,
                    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _screen->root,
                    XCB_NONE, XCB_BUTTON_INDEX_5, XCB_NONE);
  }

  void WindowManagers::Focus(Client::Client *client)
  {

    if (_clientManager->GetCurrentClient() && _clientManager->GetCurrentClient() != client)
      UnFocus(client);

    if (client)
    {
      xcb_set_input_focus(_connection, XCB_INPUT_FOCUS_POINTER_ROOT, client->GetWindow(), XCB_CURRENT_TIME);
      xcb_ewmh_set_active_window(_ewmh, 0, client->GetWindow());
      SendEvent(client->GetWindow(), _ewmh->WM_PROTOCOLS, XCB_EVENT_MASK_NO_EVENT, _WM_TAKE_FOCUS, XCB_CURRENT_TIME, 0, 0, 0);

      uint32_t col = Utils::ColorManager::RGB(152, 10, 223);
      xcb_change_window_attributes(_connection, client->GetWindow(), XCB_CW_BORDER_PIXEL, &col);
    }
    else
    {
      // std::cout << "focus to root" << std::endl;
      // xcb_set_input_focus(_connection, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_NONE, XCB_CURRENT_TIME);
      // xcb_delete_property(_connection, _screen->root, _ewmh->_NET_ACTIVE_WINDOW);

      // if (_clientManager->GetCurrentClient())
      // {
      //   uint32_t col = Utils::ColorManager::RGB(55, 182, 12);
      //   xcb_change_window_attributes(_connection, _clientManager->GetCurrentClient()->GetWindow(), XCB_CW_BORDER_PIXEL, &col);
      // }
    }

    _clientManager->SetCurrentClient(client);
    xcb_flush(_connection);
  }

  void WindowManagers::UnFocus(Client::Client *client)
  {
    if (!client)
      return;

    xcb_ungrab_button(_connection, XCB_BUTTON_INDEX_ANY, client->GetWindow(), XCB_MOD_MASK_ANY);
    uint32_t col = Utils::ColorManager::RGB(152, 213, 98);
    xcb_change_window_attributes(_connection, client->GetWindow(), XCB_CW_BORDER_PIXEL, &col);
  }

  bool WindowManagers::SendEvent(
      xcb_window_t win, xcb_atom_t proto, int mask, long d0, long d1, long d2, long d3, long d4)
  {

    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = proto;
    event.data.data32[0] = d0;
    event.data.data32[1] = d1;
    event.data.data32[2] = d2;
    event.data.data32[3] = d3;
    event.data.data32[4] = d4;

    xcb_send_event(_connection, 0, win, mask, (const char *)&event);
    xcb_flush(_connection);

    return false;
  }

  void WindowManagers::Quit()
  {
    isRunning = false;
    _clientManager->ClearClient();
  }
}