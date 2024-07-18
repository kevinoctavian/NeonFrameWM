#pragma once

#include "Client.hpp"

#include <xcb/xcb.h>
#include <vector>

namespace NFWM
{
  class WindowManagers;
  namespace Client
  {
    class ClientManager
    {
    private:
      WindowManagers *_wm;
      xcb_connection_t *_conn;
      int _desktopNumber;

      std::vector<Client *> _clients;
      Client *_currentClient;

    public:
      ClientManager(WindowManagers *wm, int desktopNumber);
      ~ClientManager();

      void AddClient(xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry);
      void RemoveClient(xcb_window_t win);
      void ClearClient();
      Client *GetClient(xcb_window_t win);

      Client *GetCurrentClient() { return _currentClient; }
      void SetCurrentClient(Client *client) { _currentClient = client; }
      size_t Size() { return _clients.size(); }

      // list all windows
      std::vector<xcb_window_t> GetAllWindows();
    };
  }

} // namespace NFWM
