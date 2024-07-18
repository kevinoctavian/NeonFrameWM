#pragma once

#include "../ShortcutHandler.hpp"
#include "../Constants/keysym.hpp"
#include "../Constants/XF86keysym.hpp"

#include <xcb/xcb.h>

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

struct KeyboardShortcut_t
{
  /**
   * modifiers is a mask for (ctrl, shift, super, alt) keys
   */
  uint16_t modifiers;

  /**
   * keysym is a mask for keys like q,w,e,r,t,y so on
   */
  xcb_keysym_t keysym;

  /**
   * func is a function that handle this shortcut
   */
  NFWM::ShortcutHandler_t func;

  /**
   * args is a variant of (int, float, and std::string)
   * so we can use three typedata in single variable
   */
  Varians_t args;
};

#define SPWNR(cmd) "/bin/sh -c " #cmd

// clang-format off
/**
 * Shortcut list area
 */
static const KeyboardShortcut_t NFWM_SHORTCUT_LIST[] = {
    /* MODIFIER,      KEYSYM,       FUNC,                     ARGS */
    { NFK_SUPER_CTRL, XK_c, &NFWM::ShortcutHandler::QuitHandler,  0 },
    { NFK_SUPER, XK_t, &NFWM::ShortcutHandler::SpawnHandler, SPWNR("kitty") },
    { NFK_SUPER_CTRL, XK_s, &NFWM::ShortcutHandler::SpawnHandler, SPWNR("rofi -show drun -show-icons") },
    { NFK_CTRL, XK_p, &NFWM::ShortcutHandler::SpawnHandler, SPWNR("echo memek") },
    { NFK_CTRL, XK_a, &NFWM::ShortcutHandler::TestHandler, 0 },
    { NFK_SUPER, XK_q, &NFWM::ShortcutHandler::KillClientHandler, 0}
};

// clang-format on
