#include "Cursor.hpp"

#include <iostream>

using namespace NFWM::Utils;

xcb_cursor_t Cursor::get_cursor(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t win, const char *cursor_id)
{
  xcb_cursor_context_t *cursor_ctx = nullptr;
  if (xcb_cursor_context_new(conn, screen, &cursor_ctx) < 0)
  {
    std::cerr << "Error when init cursor context" << std::endl;
    return XCB_NONE;
  }

  xcb_cursor_t cursor = xcb_cursor_load_cursor(cursor_ctx, cursor_id);

  if (cursor == 0)
  {
    cursor = xcb_cursor_load_cursor(cursor_ctx, "left_ptr");
  }

  xcb_cursor_context_free(cursor_ctx);
  return cursor;
}

void Cursor::create_cursor(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t win, const char *cursor_id)
{
  xcb_cursor_t cursor = get_cursor(conn, screen, win, cursor_id);
  xcb_change_window_attributes(conn, win, XCB_CW_CURSOR, &cursor);
}

void Cursor::create_cursor(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t win, int cursor_id)
{
  uint32_t values_list[3];
  xcb_void_cookie_t fontCookie;
  xcb_void_cookie_t gcCookie;
  xcb_font_t font;
  xcb_cursor_t cursor;
  xcb_gcontext_t gc;
  uint32_t mask;
  uint32_t value_list;

  xcb_generic_error_t *err;

  // generate font
  font = xcb_generate_id(conn);
  fontCookie = xcb_open_font_checked(conn, font, 6, "cursor");

  err = xcb_request_check(conn, fontCookie);
  if (err)
  {
    std::cout << "ERROR: can't open font : " << err->error_code << std::endl;
    return;
  }

  cursor = xcb_generate_id(conn);
  xcb_create_glyph_cursor(
      conn, cursor, font, font,
      cursor_id, cursor_id + 1,
      0, 0, 0,
      65535, 65535, 65535);

  gc = xcb_generate_id(conn);
  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
  values_list[0] = screen->black_pixel;
  values_list[1] = screen->white_pixel;
  values_list[2] = font;
  gcCookie = xcb_create_gc_checked(conn, gc, win, mask, values_list);
  err = xcb_request_check(conn, gcCookie);
  if (err)
  {
    std::cout << "ERROR: can't create gc : " << err->error_code << std::endl;
    return;
  }

  mask = XCB_CW_CURSOR;
  value_list = cursor;
  xcb_change_window_attributes(conn, win, mask, &value_list);

  xcb_free_cursor(conn, cursor);
  xcb_free_gc(conn, gc);

  fontCookie = xcb_close_font_checked(conn, font);
  err = xcb_request_check(conn, fontCookie);
  if (err)
  {
    std::cout << "ERROR: can't clone font : " << err->error_code << std::endl;
    return;
  }
}
