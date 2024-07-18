#pragma once

#include <xcb/xcb.h>
#include <string>

namespace NFWM
{
  class WindowManagers;
  namespace Client
  {
    enum ClientState
    {
      IsFixed = 1 << 0,
      IsCentered,
      IsFloating = 1 << 2,
      IsUrgent = 1 << 3,
      NeverFocus = 1 << 4,
      IsFullscreen = 1 << 5,
      Killed = 1 << 6
    };

    class Client
    {
    private:
      WindowManagers *_wm;
      xcb_connection_t *_conn;
      xcb_window_t _win;
      int _desktopNumber;

      // Client Geometry
      int16_t _x, _y, _oldX, _oldY;
      uint16_t _w, _h, _oldW, _oldH;
      uint32_t _borderWidth, _oldBorderWidth;

      // Client tags
      const char *_name;
      uint32_t _tags;
      bool _isHidden;
      // TODO: Client icons
      uint8_t _state, _oldState;
      bool _isMoved;

    public:
      Client(WindowManagers *wm, xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry, int desktopNumber);
      ~Client();

      // Action
      bool IsMoved() { return _isMoved; }
      void Kill(bool force = false);
      void Show();
      void Hide();

      // Getter
      const char *GetName() { return _name; }
      uint8_t GetState() { return _state; }
      uint32_t GetTags() { return _tags; }
      xcb_window_t GetWindow() { return _win; }
      bool IsHidden() { return _isHidden; }

      // Setter
      void SetName(const char *name) { _name = name; }
      void SetState(uint8_t _state);
      void SetTags(uint32_t tags) { _tags = tags; }
      void SetMoved(bool isMoved) { _isMoved = isMoved; }
    };
  }
}