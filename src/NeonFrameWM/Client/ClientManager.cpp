#include "ClientManager.hpp"

#include "../WindowManagers.hpp"
#include "../Utils/Log.hpp"
#include "../Constants/mask.hpp"

using namespace NFWM::Client;

ClientManager::ClientManager(WindowManagers *wm) : _wm(wm)
{
  _conn = _wm->GetConnection();
}

ClientManager::~ClientManager()
{
  if (!_clients.empty())
  {
    for (auto *client : _clients)
    {
      client->Kill(true);
    }
  }
}

void ClientManager::AddClient(xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry)
{
  if (GetClient(win) != nullptr)
    return;

  int padding = 4;
  uint32_t screenWidth = _wm->GetScreen()->width_in_pixels;
  uint32_t screenHeight = _wm->GetScreen()->height_in_pixels;

  geometry->border_width = 3;
  geometry->x = padding;
  geometry->y = padding;
  geometry->width = screenWidth - (padding * 2) - geometry->border_width * 2;
  geometry->height = screenHeight - (padding * 2) - geometry->border_width * 2;

  uint32_t mask;
  uint32_t values[5];
  mask = WIN_MOVE_RESIZE_MASK | XCB_CONFIG_WINDOW_BORDER_WIDTH;
  values[0] = geometry->x;            // X
  values[1] = geometry->y;            // Y
  values[2] = geometry->width;        // W
  values[3] = geometry->height;       // H
  values[4] = geometry->border_width; // Border Width
  xcb_configure_window(_conn, win, mask, values);

  mask = XCB_CW_EVENT_MASK;
  values[0] = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes(_conn, win, mask, values);

  xcb_map_window(_conn, win);
  xcb_flush(_conn);

  Client *client = new Client(_wm, win, attr, geometry);

  _clients.push_back(client);
  _wm->Focus(client);
  xcb_flush(_conn);
}

void ClientManager::RemoveClient(xcb_window_t win)
{
  Client *client = GetClient(win);

  if (client)
  {
    DEBUG_LOG("Removing Client %d == %d from list", client->GetWindow(), win)
    client->Kill();
    _clients.erase(
        std::remove(_clients.begin(), _clients.end(), client),
        _clients.end());

    if (!_clients.empty())
    {
      _currentClient = _clients.front();
      _wm->Focus(_currentClient);
    }
    else
    {
      _currentClient = nullptr;
    }
    delete client;
  }
}

void ClientManager::ClearClient()
{
  if (_clients.empty())
    return;

  for (auto *client : _clients)
  {
    DEBUG_LOG("clearing client %d", client->GetWindow())
    client->Kill();
    xcb_flush(_conn);
    delete client;
  }

  _clients.clear();
}

Client *ClientManager::GetClient(xcb_window_t win)
{
  if (_clients.empty())
    return nullptr;

  for (auto *client : _clients)
  {
    // std::cout << client->GetWindow() << " " << win << std::endl;
    if (win == client->GetWindow())
      return client;
  }

  return nullptr;
}