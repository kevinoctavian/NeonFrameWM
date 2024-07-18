#pragma once

#include <string>
#include <variant>
#include <functional>
#include <iostream>

#include "Utils/ColorManager.hpp"

using Varians_t = std::variant<int, float, std::string>;
#define _SV(name) static void name##Handler(WindowManagers *wm, Varians_t params)

namespace NFWM
{
  class WindowManagers;
  using ShortcutHandler_t = std::function<void(WindowManagers *, Varians_t)>;

  class ShortcutHandler
  {
  private:
    static std::vector<const char *> Tokenize(const std::string &input);

  public:
    static void QuitHandler(WindowManagers *wm, Varians_t params);
    static void SpawnHandler(WindowManagers *wm, Varians_t params);
    static void TestHandler(WindowManagers *wm, Varians_t params);
    static void KillClientHandler(WindowManagers *wm, Varians_t params);
  };

}