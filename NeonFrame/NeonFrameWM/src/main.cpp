#include "WindowManagers.hpp"

#include "Utils/Log.hpp"

int main(int argc, char **argv)
{
  // NFWM::WindowManagers wm(":1");
  NFWM::WindowManagers wm(nullptr);

  DEBUG_LOG("Opening NeonFrame Window Manager")

  try
  {
    wm.Start();
  }
  catch (const std::exception &e)
  {
    std::cerr << "[Error]: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}