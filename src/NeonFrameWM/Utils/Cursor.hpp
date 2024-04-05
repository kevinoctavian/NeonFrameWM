#pragma once

#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>

/**
 * cursor list to make cursor lebih mudah
 */
#include "../Constants/cursorlist.hpp"

namespace NFWM::Utils
{
  namespace Cursor
  {
    void create_cursor(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t win, const char *cursor_id);
    void create_cursor(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t win, int cursor_id);
  } // namespace Cursor
}