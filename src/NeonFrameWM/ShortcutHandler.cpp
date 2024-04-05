#include "ShortcutHandler.hpp"
#include "WindowManagers.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <xcb/xcb.h>

using namespace NFWM;

// Void ShortcutHanlder (vsh)
// #define _VSH(name) void ShortcutHandler::name##Handler(WindowManagers *wm, Varians_t params)

std::vector<const char *> ShortcutHandler::Tokenize(const std::string &input)
{
  std::vector<const char *> tokens;
  std::string token;
  bool insideQuotes = false;

  for (char c : input)
  {
    if (c == '"')
    {
      insideQuotes = !insideQuotes;
    }
    else if (c == ' ' && !insideQuotes)
    {
      if (!token.empty())
      {
        tokens.push_back(strdup(token.c_str()));
        token.clear();
      }
    }
    else
    {
      token += c;
    }
  }

  if (!token.empty())
  {
    tokens.push_back(strdup(token.c_str()));
  }

  tokens.push_back(nullptr);

  return tokens;
}

void ShortcutHandler::QuitHandler(WindowManagers *wm, Varians_t params)
{
  wm->Quit();

  // make sure main loop run once
  xcb_generic_event_t ev = {};
  ev.response_type = XCB_NO_OPERATION;
  xcb_send_event(wm->GetConnection(), 0, wm->GetScreen()->root, XCB_EVENT_MASK_NO_EVENT, reinterpret_cast<const char *>(&ev));
}

void ShortcutHandler::SpawnHandler(WindowManagers *wm, Varians_t params)
{
  std::string path = std::get<std::string>(params);
  std::vector<const char *> pathToken = Tokenize(path);
  pid_t pid = fork();

  if (pid == -1)
  {
    std::cerr << "Failed to fork process." << std::endl;
    return;
  }
  else if (pid == 0)
  {
    if (wm->GetConnection())
    {
      close(xcb_get_file_descriptor(wm->GetConnection()));
    }
    setsid();
    char **args = const_cast<char **>(pathToken.data());

    if (execvp(args[0], args) == -1)
    {
      std::cout << "Failed to execute command: " << path << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << "success to execute command: " << path << std::endl;
    exit(EXIT_SUCCESS);
  }
}

void ShortcutHandler::TestHandler(WindowManagers *wm, Varians_t variant)
{

  std::cout << "berhasil ";

  uint32_t mask = XCB_CW_BACK_PIXEL;
  // uint32_t values = Utils::ColorManager::RGB(248, 255, 0);
  uint32_t values = Utils::ColorManager::RGB(rand() % 256, rand() % 256, rand() % 256);

  xcb_change_window_attributes(wm->GetConnection(), wm->GetScreen()->root, mask, &values);

  xcb_clear_area(wm->GetConnection(), 0, wm->GetScreen()->root, 0, 0, wm->GetScreen()->width_in_pixels, wm->GetScreen()->height_in_pixels);
  xcb_flush(wm->GetConnection());

  std::cout << "Curent client total " << wm->GetClientManager()->Size() << std::endl;
  wm->GetClientManager()->GetCurrentClient()->Show();
}

void ShortcutHandler::KillClientHandler(WindowManagers *wm, Varians_t variant)
{
  Client::Client *client = wm->GetClientManager()->GetCurrentClient();
  std::cout << "Trying to kill client " << (client ? client->GetWindow() : 0) << std::endl;

  if (client)
  {
    std::cout << "Trying to kill client " << client->GetWindow() << std::endl;

    wm->GetClientManager()->RemoveClient(client->GetWindow());
  }
}
