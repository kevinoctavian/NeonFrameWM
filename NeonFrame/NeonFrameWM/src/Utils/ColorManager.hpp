#pragma once

#include <stdbit.h>
#include <xcb/xcb.h>

namespace NFWM::Utils
{
  namespace ColorManager
  {

    uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);

    xcb_alloc_color_reply_t *RGBA_Colormap(xcb_connection_t *conn, xcb_screen_t *screen, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    xcb_alloc_color_reply_t *RGB_Colormap(xcb_connection_t *conn, xcb_screen_t *screen, uint8_t r, uint8_t g, uint8_t b);

  } // namespace ColorManager

} // namespace NFWM::Utils
