#include "Client.hpp"

#include "../WindowManagers.hpp"
#include "../Utils/Log.hpp"

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
using namespace NFWM::Client;

Client::Client(WindowManagers *wm, xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry, int desktopNumber) : _wm(wm), _win(win), _desktopNumber(desktopNumber)
{
  _x = _oldX = geometry->x;
  _y = _oldY = geometry->y;
  _w = _oldW = geometry->width;
  _h = _oldH = geometry->height;
  _oldBorderWidth = geometry->border_width;

  _conn = wm->GetConnection();

  xcb_get_property_cookie_t winNameCookie = xcb_icccm_get_wm_name(_conn, win);
  xcb_icccm_get_text_property_reply_t winName;
  xcb_icccm_get_wm_name_reply(_conn, winNameCookie, &winName, nullptr);

  _name = winName.name;
  DEBUG_LOG("Name %s (%s)", _name, winName.name);
  _isHidden = false;
}

Client::~Client()
{
}

void Client::Kill(bool force)
{
  bool supports_protocol;

  xcb_ewmh_connection_t *ewmh = _wm->GetEWMH();

  xcb_atom_t wm_protocols_atom = ewmh->WM_PROTOCOLS;
  xcb_atom_t wm_delete_window_atom = _wm->GetDeleteWindowAtom();

  xcb_get_property_cookie_t cookie = xcb_get_property(_conn, 0, _win, wm_protocols_atom, XCB_ATOM_ATOM, 0, 1024);
  xcb_get_property_reply_t *reply_prop = xcb_get_property_reply(_conn, cookie, NULL);
  if (reply_prop)
  {
    xcb_atom_t *protocols = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(reply_prop));
    int num_protocols = xcb_get_property_value_length(reply_prop) / sizeof(xcb_atom_t);
    supports_protocol = false;
    for (int i = 0; i < num_protocols; ++i)
    {
      if (protocols[i] == wm_delete_window_atom)
      {
        supports_protocol = true;
        break;
      }
    }
    free(reply_prop);
  }

  if (supports_protocol)
  {
    DEBUG_LOG("Kill Client id %d (%s) with protocol", _win, _name);
    force = false;

    xcb_client_message_event_t msg;
    msg.response_type = XCB_CLIENT_MESSAGE;
    msg.window = _win;
    msg.format = 32;
    msg.type = wm_protocols_atom;

    msg.data.data32[0] = wm_delete_window_atom;
    msg.data.data32[1] = XCB_CURRENT_TIME;
    msg.data.data32[2] = 0;
    msg.data.data32[3] = 0;
    msg.data.data32[4] = 0;

    xcb_send_event(_conn, 0, _win, XCB_EVENT_MASK_NO_EVENT, (const char *)&msg);
    xcb_flush(_conn);
  }
  else
    force = true;

  if (force)
  {
    DEBUG_LOG("Kill Client id %d (%s) with force", _win, _name);

    xcb_grab_server(_conn);
    xcb_set_close_down_mode(_conn, XCB_CLOSE_DOWN_DESTROY_ALL);
    xcb_kill_client(_conn, _win);
    xcb_flush(_conn);
    xcb_ungrab_server(_conn);
  }

  SetState(Killed);
}

void Client::SetState(uint8_t state)
{
  if (state & ClientState::IsFullscreen)
  {
    _state = IsFullscreen;
    return;
  }

  if (state & ClientState::Killed)
  {
    _state = Killed;
    return;
  }

  _state = state;
}

void Client::Show()
{
  xcb_get_window_attributes_reply_t *attrReply;
  {
    xcb_get_window_attributes_cookie_t attrCookie = xcb_get_window_attributes(_conn, _win);
    attrReply = xcb_get_window_attributes_reply(_conn, attrCookie, nullptr);
  }

  if (!attrReply)
  {
    DEBUG_LOG("attrReply is empty")
    return;
  }

  if (attrReply->map_state == XCB_MAP_STATE_UNMAPPED)
  {
    xcb_map_window(_conn, _win);
    _isHidden = false;
  }

  free(attrReply);
}

void Client::Hide()
{
  xcb_get_window_attributes_reply_t *attrReply;
  {
    xcb_get_window_attributes_cookie_t attrCookie = xcb_get_window_attributes(_conn, _win);
    attrReply = xcb_get_window_attributes_reply(_conn, attrCookie, nullptr);
  }

  if (!attrReply)
  {
    DEBUG_LOG("attrReply is empty")
    return;
  }

  if (attrReply->map_state == XCB_MAP_STATE_VIEWABLE)
  {
    xcb_unmap_window(_conn, _win);
    _isHidden = true;
  }

  free(attrReply);
}
