#pragma once

#include <xcb/xcb.h>
#include <vector>
#include <string>
#include <cstdlib>

namespace NFWM
{

  namespace NF_Image
  {
    enum BackgroundType
    {
      SCALED,
      CENTERED,
      TILED,
      ZOOMED,
      ZOOMEDFILL
    };

    enum FileType
    {
      PNG,
      JPG
    };

    class BackgroundManager
    {
    private:
      xcb_connection_t *_connection;

      // Image needed
      int _width, _height;
      std::vector<u_char> _imageData;

      xcb_gcontext_t _gc;
      xcb_pixmap_t _pixmap;
      int _winWidth, _winHeight;
      u_int8_t _depth;
      xcb_window_t _window;

      std::string _imageFile;

      bool _needReload;

    private:
      bool LoadPng();
      bool LoadJpeg();

      void ResizeImage();
      void ScaledImage();

    public:
      BackgroundManager(xcb_connection_t *connection, std::string imageFile, xcb_window_t window, u_int8_t depth);
      ~BackgroundManager();

      void DrawBackground(bool isRedraw, BackgroundType type);

      // setter
      void SetWindow(xcb_window_t win);
      void SetImageFile(std::string imageFile);
    };
  } // namespace NF_Image

} // namespace NFWM
