#include "ColorManager.hpp"

#include <iostream>

namespace NFWM::Utils
{
  namespace ColorManager
  {

    uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
      // std::cout << ((a << 24) | (r << 16) | (g << 8) | b) << std::endl;

      return (a << 24) | (r << 16) | (g << 8) | b;
    }

    uint32_t RGB(uint8_t r, uint8_t g, uint8_t b)
    {
      return RGBA(r, g, b, 255);
    }

    xcb_alloc_color_reply_t *RGBA_Colormap(xcb_connection_t *conn, xcb_screen_t *screen, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
      xcb_colormap_t colormap = xcb_generate_id(conn);
      xcb_create_colormap(conn, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, screen->root_visual);

      xcb_alloc_color_reply_t *reply = xcb_alloc_color_reply(conn, xcb_alloc_color(conn, colormap, r * 257, g * 257, b * 257), NULL);
      if (reply == nullptr)
      {
        // Handle error
        return nullptr;
      }

      if (a != 255)
      {
        uint32_t pixel = reply->pixel;

        // Create alpha mask
        uint32_t alpha_mask = (a * 0x10000) / 256;
        pixel = (pixel & 0x00FFFFFF) | (alpha_mask << 24);

        reply->pixel = pixel;
      }

      xcb_free_colormap(conn, colormap);
      return reply;
    }

    xcb_alloc_color_reply_t *RGB_Colormap(xcb_connection_t *conn, xcb_screen_t *screen, uint8_t r, uint8_t g, uint8_t b)
    {
      return RGBA_Colormap(conn, screen, r, g, b, 255);
    }

  } // namespace ColorManager

} // namespace NFWM::Utils