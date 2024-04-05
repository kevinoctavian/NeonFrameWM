#include "NeonFrameWM/WindowManagers.hpp"

#include "NeonFrameWM/Utils/Log.hpp"

int main(int argc, char **argv)
{
  // NFWM::WindowManagers wm(":1");
  NFWM::WindowManagers wm(nullptr);

  DEBUG_LOG("Opening NeonFrame Window Manager")

  return wm.Start();
}