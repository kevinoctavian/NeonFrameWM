#include "EventManager.hpp"
#include "WindowManagers.hpp"

#include "Constants/mask.hpp"
#include "Constants/keysym.hpp"
#include "Utils/Log.hpp"

#include <stdexcept>
#include <iostream>

#include <xcb/xcb_event.h>

#include <xcb/shape.h>

using namespace NFWM;

EventManager::EventManager(WindowManagers *wm) : _wm(wm)
{
  if (!wm)
  {
    throw std::runtime_error("EventManager doesn't accept null wm");
  }

  _conn = _wm->GetConnection();
  keySymbol = xcb_key_symbols_alloc(_conn);
}

EventManager::~EventManager()
{
  xcb_key_symbols_free(keySymbol);
}

int EventManager::RecieveEvents(xcb_generic_event_t *ev)
{
  // DEBUG_LOG("event type : %d", ev->response_type)
  xcb_generic_error_t *error;

  switch (XCB_EVENT_RESPONSE_TYPE(ev))
  {
  case 0x00:
    error = reinterpret_cast<xcb_generic_error_t *>(ev);
    ERROR_LOG("Error from event loop: %d with type %s", error->error_code, xcb_event_get_error_label(error->error_code))
    break;

    /**
     * Keyboard Event catch
     */
  case XCB_KEY_PRESS:
    KeyboardHandler(reinterpret_cast<xcb_key_press_event_t *>(ev));
    break;

    /**
     * Pointer Event
     */
  case XCB_MOTION_NOTIFY:
    MotionHandler(reinterpret_cast<xcb_motion_notify_event_t *>(ev));
    break;
  case XCB_BUTTON_PRESS:
    ButtonPressHandler(reinterpret_cast<xcb_button_press_event_t *>(ev));
    break;
  case XCB_BUTTON_RELEASE:
    ButtonReleaseHandler(reinterpret_cast<xcb_button_release_event_t *>(ev));
    break;

    /**
     * Structure control events
     */
  case XCB_CONFIGURE_REQUEST:
    ConfigureReqHandler(reinterpret_cast<xcb_configure_request_event_t *>(ev));
    xcb_flush(_conn);
    break;
  case XCB_MAP_REQUEST:
    MapReqHandler(reinterpret_cast<xcb_map_request_event_t *>(ev));
    xcb_flush(_conn);
    break;
  case XCB_RESIZE_REQUEST:
    ResizeReqHandler(reinterpret_cast<xcb_resize_request_event_t *>(ev));
    xcb_flush(_conn);
    break;

    /**
     * Window state notification events
     */
  case XCB_CONFIGURE_NOTIFY:
    ConfigureNotifyHandler(reinterpret_cast<xcb_configure_notify_event_t *>(ev));
    break;
  case XCB_CREATE_NOTIFY:
    CreateNotifyHandler(reinterpret_cast<xcb_create_notify_event_t *>(ev));
    break;
  case XCB_DESTROY_NOTIFY:
    DestroyNotifyHandler(reinterpret_cast<xcb_destroy_notify_event_t *>(ev));
    break;
  case XCB_MAP_NOTIFY:
    MapNotifyHandler(reinterpret_cast<xcb_map_notify_event_t *>(ev));
    break;
  case XCB_MAPPING_NOTIFY:
    MappingNotifyHandler(reinterpret_cast<xcb_mapping_notify_event_t *>(ev));
    break;
  case XCB_UNMAP_NOTIFY:
    UnMapNotifyHandler(reinterpret_cast<xcb_unmap_notify_event_t *>(ev));
    break;
  case XCB_REPARENT_NOTIFY:
    ReparentNotifyHandler(reinterpret_cast<xcb_reparent_notify_event_t *>(ev));
    break;
  case XCB_VISIBILITY_NOTIFY:
    VisibilityNotifyHandler(reinterpret_cast<xcb_visibility_notify_event_t *>(ev));
    break;

    /**
     * Client Comunication Event
     */
  case XCB_CLIENT_MESSAGE:
    ClientMessageHandler(reinterpret_cast<xcb_client_message_event_t *>(ev));
    break;

    /**
     * Exposure Event
     */
  case XCB_EXPOSE:
    ExposeHandler(reinterpret_cast<xcb_expose_event_t *>(ev));
    break;

    /**
     * Window crossing events
     */
  case XCB_ENTER_NOTIFY:
    EnterNotifyHandler(reinterpret_cast<xcb_enter_notify_event_t *>(ev));
    break;
  case XCB_LEAVE_NOTIFY:
    LeaveNotifyHandler(reinterpret_cast<xcb_leave_notify_event_t *>(ev));
    break;

    /**
     * Input focus events
     */
  case XCB_FOCUS_IN:
    FocusInHandler(reinterpret_cast<xcb_focus_in_event_t *>(ev));
    break;
  case XCB_FOCUS_OUT:
    FocusOutHandler(reinterpret_cast<xcb_focus_out_event_t *>(ev));
    break;
  }
  return 0;
}

// Events Function
// Keyboard handler
void EventManager::KeyboardHandler(xcb_key_press_event_t *ev)
{
  xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keySymbol, ev->detail, 0);
  uint16_t state = ev->state;

  DEBUG_LOG("Keysym %d + %d from %d", keysym, state, ev->event)
  // std::cout << "from window id: " << ev->event << " root id: " << ev->root << std::endl;

  _wm->GetShortcutManager()->HandleShortcut(state, keysym);
}

// Mouse Handler
void EventManager::MotionHandler(xcb_motion_notify_event_t *ev) {}
void EventManager::ButtonPressHandler(xcb_button_press_event_t *ev)
{
  DEBUG_LOG("Pressed Button type: %d", ev->detail)
}
void EventManager::ButtonReleaseHandler(xcb_button_release_event_t *ev)
{
  DEBUG_LOG("Released Button type: %d", ev->detail)
}

// Structure control handler
void EventManager::ConfigureReqHandler(xcb_configure_request_event_t *ev)
{
  DEBUG_LOG("Configure Request from %d (%d)", ev->window, ev->parent)

  xcb_configure_window_value_list_t winVal;
  if (_wm->GetClientManager()->GetClient(ev->window))
  {
  }
  else
  {
    winVal.x = ev->x;
    winVal.y = ev->y;
    winVal.width = ev->width;
    winVal.height = ev->height;
    winVal.border_width = ev->border_width;
    winVal.sibling = ev->sibling;
    winVal.stack_mode = ev->stack_mode;

    xcb_configure_window_aux(_conn, ev->window, ev->value_mask, &winVal);
  }
}
void EventManager::MapReqHandler(xcb_map_request_event_t *ev)
{
  DEBUG_LOG("Map Request from %d (%d)", ev->window, ev->parent)

  xcb_get_window_attributes_cookie_t winAtCookie = xcb_get_window_attributes(_conn, ev->window);
  xcb_get_geometry_cookie_t winGeomCookie = xcb_get_geometry(_conn, ev->window);

  xcb_get_window_attributes_reply_t *attrReply = xcb_get_window_attributes_reply(_conn, winAtCookie, nullptr);

  if (!attrReply)
  {
    std::cout << "error from window id : " << ev->window << " ";
    std::cout << "attribute empty or window is removed" << std::endl;
  }
  else if (attrReply->override_redirect)
    return;

  xcb_get_geometry_reply_t *geomReply = xcb_get_geometry_reply(_conn, winGeomCookie, nullptr);

  if (!geomReply)
  {
    std::cout << "error from window id :" << ev->window;
    std::cout << "geometry empty or window is removed" << std::endl;
    return;
  }

  DEBUG_LOG("Map State %d", attrReply->map_state)

  // TODO: add client to wm list
  _wm->GetClientManager()->AddClient(ev->window, attrReply, geomReply);

  // _wm->InitShortcut();

  free(attrReply);
  free(geomReply);
}
void EventManager::ResizeReqHandler(xcb_resize_request_event_t *ev)
{
  DEBUG_LOG("Resize Request from %d (%dx%d)", ev->window, ev->width, ev->height)

  uint16_t values[2] = {
      ev->width,
      ev->height,
  };

  xcb_configure_window(_conn, ev->window, WIN_RESIZE_MASK, values);
}

// Window state notification handler
void EventManager::ConfigureNotifyHandler(xcb_configure_notify_event_t *ev) {}
void EventManager::CreateNotifyHandler(xcb_create_notify_event_t *ev) {}
void EventManager::DestroyNotifyHandler(xcb_destroy_notify_event_t *ev)
{
  DEBUG_LOG("Destroy notify from %d (%d)", ev->window, ev->event)

  _wm->GetClientManager()->RemoveClient(ev->window);
}
void EventManager::MapNotifyHandler(xcb_map_notify_event_t *ev)
{

  // xcb_query_tree_reply_t *tr = xcb_query_tree_reply(_conn, xcb_query_tree(_conn, ev->window), nullptr);
  // if (!tr)
  // {
  //   std::cerr << "Error: Failed to query tree for window " << ev->window << std::endl;
  //   return;
  // }
  std::cout << "map window id: " << ev->window << " from screen successfully" << std::endl;

  // free(tr);
}
void EventManager::MappingNotifyHandler(xcb_mapping_notify_event_t *ev)
{
  DEBUG_LOG("Mapping Notify")

  xcb_refresh_keyboard_mapping(keySymbol, ev);
  if (ev->response_type == XCB_MAPPING_KEYBOARD)
  {
    _wm->GrabKey();
  }
}
void EventManager::UnMapNotifyHandler(xcb_unmap_notify_event_t *ev)
{
  Client::Client *cl = _wm->GetClientManager()->GetClient(ev->window);

  DEBUG_LOG("UnMap Notify from %d (%d)", ev->window, ev->event)

  if (cl)
  {
  }
}
void EventManager::ReparentNotifyHandler(xcb_reparent_notify_event_t *ev) {}
void EventManager::VisibilityNotifyHandler(xcb_visibility_notify_event_t *ev) {}

// Client message handler
void EventManager::ClientMessageHandler(xcb_client_message_event_t *ev)
{
  DEBUG_LOG("Client Message from window %d type %d", ev->window, ev->type)

  ev->type;
}

// Exposure handler
void EventManager::ExposeHandler(xcb_expose_event_t *ev)
{
  std::cout << "Expose Event, redraw background" << std::endl;
}

// Window crossing handler
void EventManager::EnterNotifyHandler(xcb_enter_notify_event_t *ev)
{
  if ((ev->mode != XCB_NOTIFY_MODE_NORMAL || ev->detail == XCB_NOTIFY_DETAIL_INFERIOR) && ev->event != ev->root)
    return;

  Client::Client *client = _wm->GetClientManager()->GetClient(ev->event);
  // std::cout << "search for " << ev->event << " root id: " << ev->root << (client == nullptr ? " not found" : " found") << std::endl;

  if (ev->event == ev->root)
  {
    // client->Kill();
    // _wm->GetClientManager()->RemoveClient(ev->event);
    // _wm->Focus(nullptr);
  }

  if (client)
  {
    // std::cout << "focusing to " << ev->event << std::endl;
    _wm->Focus(client);
  }
}
void EventManager::LeaveNotifyHandler(xcb_leave_notify_event_t *ev) {}

// Input focus handler
void EventManager::FocusInHandler(xcb_focus_in_event_t *ev)
{
  std::cout << "Focus in" << std::endl;

  Client::ClientManager *clMan = _wm->GetClientManager();

  if (clMan->GetCurrentClient() && clMan->GetCurrentClient()->GetWindow() != ev->event)
  {
    _wm->Focus(clMan->GetCurrentClient());
  }
}
void EventManager::FocusOutHandler(xcb_focus_out_event_t *ev) {}