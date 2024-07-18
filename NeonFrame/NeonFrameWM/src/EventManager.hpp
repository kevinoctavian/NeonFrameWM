#pragma once

#include <xcb/xcb_keysyms.h>
#include <xcb/xcb.h>

#include "Client/DesktopManager.hpp"

namespace NFWM
{
  class WindowManagers;
  class EventManager
  {
  private:
    WindowManagers *_wm;
    Desktop::DesktopManager *_desktopManager;

    xcb_connection_t *_conn;
    xcb_key_symbols_t *keySymbol;

    // Events Function
    // Keyboard handler
    void KeyboardHandler(xcb_key_press_event_t *ev);

    // Mouse Handler
    void MotionHandler(xcb_motion_notify_event_t *ev);
    void ButtonPressHandler(xcb_button_press_event_t *ev);
    void ButtonReleaseHandler(xcb_button_release_event_t *ev);

    // Structure control handler
    void ConfigureReqHandler(xcb_configure_request_event_t *ev);
    void MapReqHandler(xcb_map_request_event_t *ev);
    void ResizeReqHandler(xcb_resize_request_event_t *ev);

    // Window state notification handler
    void ConfigureNotifyHandler(xcb_configure_notify_event_t *ev);
    void CreateNotifyHandler(xcb_create_notify_event_t *ev);
    void DestroyNotifyHandler(xcb_destroy_notify_event_t *ev);
    void MapNotifyHandler(xcb_map_notify_event_t *ev);
    void MappingNotifyHandler(xcb_mapping_notify_event_t *ev);
    void UnMapNotifyHandler(xcb_unmap_notify_event_t *ev);
    void ReparentNotifyHandler(xcb_reparent_notify_event_t *ev);
    void VisibilityNotifyHandler(xcb_visibility_notify_event_t *ev);

    // Client message handler
    void ClientMessageHandler(xcb_client_message_event_t *ev);

    // Exposure handler
    void ExposeHandler(xcb_expose_event_t *ev);

    // Window crossing handler
    void EnterNotifyHandler(xcb_enter_notify_event_t *ev);
    void LeaveNotifyHandler(xcb_leave_notify_event_t *ev);

    // Input focus handler
    void FocusInHandler(xcb_focus_in_event_t *ev);
    void FocusOutHandler(xcb_focus_out_event_t *ev);

    // Property Notify Handler
    void PropertyNotifyHandler(xcb_property_notify_event_t *ev);

  public:
    EventManager(WindowManagers *wm);
    ~EventManager();

    int RecieveEvents(xcb_generic_event_t *ev);
  };
}