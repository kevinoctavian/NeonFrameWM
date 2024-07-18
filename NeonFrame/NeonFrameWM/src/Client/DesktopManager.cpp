#include "DesktopManager.hpp"
#include "../WindowManagers.hpp"

#include <xcb/xcb_ewmh.h>

#include <algorithm>
#include <string>
#include <sstream>

using namespace NFWM;
using namespace NFWM::Desktop;

int NFWM::Desktop::DesktopManager::FixIndex(int desktopIndex)
{
  return std::clamp<int>(desktopIndex, 0, _desktopCount - 1);
}

DesktopManager::DesktopManager(WindowManagers *wm, int desktopCount) : _wm(wm), _desktopCount(desktopCount)
{
  _desktops.resize(desktopCount);
  for (int i = 0; i < _desktopCount; i++)
  {
    _desktops[i] = new Client::ClientManager(wm, i);
  }
  _desktops.shrink_to_fit();

  SetCurrentDesktop(0);
}

DesktopManager::~DesktopManager()
{
  for (auto &desktop : _desktops)
  {
    delete desktop;
  }
}

void DesktopManager::SetupEWMH()
{
  xcb_ewmh_set_number_of_desktops(_wm->GetEWMH(), 0, _desktopCount);
  xcb_ewmh_set_current_desktop(_wm->GetEWMH(), 0, 0);

  std::string names;
  for (int i = 0; i < _desktopCount; i++)
  {
    std::stringstream ss;
    ss << "Desktop ";
    ss << std::to_string(i + 1);
    ss << "\0";

    names += ss.str().c_str();
  }

  xcb_ewmh_set_desktop_names(_wm->GetEWMH(), 0, names.size(), names.data());
}

// action
void DesktopManager::ClearClient()
{
  for (auto &desktop : _desktops)
  {
    desktop->ClearClient();
  }
}

void DesktopManager::ClearClient(int desktopIndex)
{
  Client::ClientManager *desktop = _desktops.at(FixIndex(desktopIndex));
  if (desktop)
  {
    desktop->ClearClient();
  }
}

void DesktopManager::AddClient(xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry)
{
  if (_currentDesktop)
  {
    _currentDesktop->AddClient(win, attr, geometry);
    SetClientLists();
  }
}

void DesktopManager::AddClient(int desktopIndex, xcb_window_t win, xcb_get_window_attributes_reply_t *attr, xcb_get_geometry_reply_t *geometry)
{
  Client::ClientManager *currentDesktop = _desktops.at(FixIndex(desktopIndex));
  if (currentDesktop)
  {
    currentDesktop->AddClient(win, attr, geometry);
    SetClientLists();
  }
}

void DesktopManager::DestroyClient(xcb_window_t win)
{
  if (_currentDesktop)
  {
    _currentDesktop->RemoveClient(win);
    SetClientLists();
  }
}

void DesktopManager::DestroyClient(int desktopIndex, xcb_window_t win)
{
  Client::ClientManager *currentDesktop = _desktops.at(FixIndex(desktopIndex));
  if (currentDesktop)
  {
    currentDesktop->RemoveClient(win);
  }
}

// getter
Client::ClientManager *DesktopManager::GetDesktop(int desktopIndex)
{
  return _desktops.at(FixIndex(desktopIndex));
}

Client::Client *DesktopManager::GetClient(xcb_window_t win)
{

  for (auto &desktop : _desktops)
  {
    Client::Client *client = desktop->GetClient(win);
    if (client)
      return client;
  }

  return nullptr;
}

// setter
void DesktopManager::SetClientLists()
{
  std::vector<xcb_window_t> windows;

  for (auto &desktop : _desktops)
  {
    std::vector<xcb_window_t> clientWindow = desktop->GetAllWindows();
    windows.insert(windows.end(), clientWindow.begin(), clientWindow.end());
  }

  xcb_ewmh_set_client_list(_wm->GetEWMH(), 0, windows.size(), windows.data());

  if (windows.empty())
  {
    xcb_ewmh_set_active_window(_wm->GetEWMH(), 0, 0);
  }
}

void DesktopManager::SetCurrentDesktop(int desktopIndex)
{
  desktopIndex = FixIndex(desktopIndex);

  _currentDesktop = _desktops.at(desktopIndex);
  _currentDesktopIndex = desktopIndex;

  xcb_ewmh_set_current_desktop(_wm->GetEWMH(), 0, 0);
}