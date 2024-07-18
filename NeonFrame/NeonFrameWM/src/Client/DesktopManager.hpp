#pragma once

#include <vector>

#include "ClientManager.hpp"

namespace NFWM
{
  class WindowManagers;
  namespace Desktop
  {
    class DesktopManager
    {
    private:
      WindowManagers *_wm;

      std::vector<Client::ClientManager *> _desktops;
      int _desktopCount;

      Client::ClientManager *_currentDesktop;
      int _currentDesktopIndex;

      // UTILITY
      int FixIndex(int desktopIndex);

    public:
      DesktopManager(WindowManagers *wm, int desktopCount);
      ~DesktopManager();

      void SetupEWMH();

      // action
      void ClearClient();
      void ClearClient(int desktopIndex);
      void AddClient(xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry);
      void AddClient(int desktopIndex, xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry);
      void DestroyClient(xcb_window_t win);
      void DestroyClient(int desktopIndex, xcb_window_t win);

      // getter
      Client::ClientManager *GetCurrentDesktop() { return _currentDesktop; }
      Client::ClientManager *GetDesktop(int desktopIndex);
      Client::Client *GetClient(xcb_window_t win);

      // setter
      void SetClientLists();
      void SetCurrentDesktop(int numberOfDesktop);
    };
  }
}