#include "NF_Image.hpp"

#include <xcb/xcb_image.h>
#include <png.h>
#include <jpeglib.h>

#include <iostream>

using namespace NFWM::NF_Image;

BackgroundManager::BackgroundManager(xcb_connection_t *connection, std::string imageFile, xcb_window_t win, u_int8_t depth) : _connection(connection), _gc(0), _pixmap(0), _depth(depth)
{
  SetWindow(win);
  SetImageFile(imageFile);
}

BackgroundManager::~BackgroundManager()
{
  xcb_free_gc(_connection, _gc);
  xcb_free_pixmap(_connection, _pixmap);
}

bool BackgroundManager::LoadPng()
{
  FILE *pngFile = fopen(_imageFile.c_str(), "rb");
  if (!pngFile)
    return false;

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png)
    return false;

  png_infop info = png_create_info_struct(png);
  if (!info)
    return false;

  if (setjmp(png_jmpbuf(png)))
    return false;

  png_init_io(png, pngFile);
  png_read_info(png, info);

  _width = png_get_image_width(png, info);
  _height = png_get_image_height(png, info);
  png_byte colorType = png_get_color_type(png, info);
  png_byte bitDepth = png_get_bit_depth(png, info);

  if (bitDepth == 16)
    png_set_strip_16(png);
  if (colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);
  if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    png_set_expand_gray_1_2_4_to_8(png);
  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);
  if (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_GRAY)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
  if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  _imageData.resize(_width * _height * 4);
  std::vector<png_bytep> rowPointer(_height);
  for (int y = 0; y < _height; y++)
  {
    rowPointer[y] = _imageData.data() + y * _width * 4;
  }

  png_read_image(png, rowPointer.data());
  fclose(pngFile);

  png_destroy_read_struct(&png, &info, nullptr);
  return true;
}

bool BackgroundManager::LoadJpeg()
{
  FILE *jpegFile = fopen(_imageFile.c_str(), "rb");
  if (!jpegFile)
    return false;

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, jpegFile);
  jpeg_read_header(&cinfo, true);
  jpeg_start_decompress(&cinfo);

  _width = cinfo.output_width;
  _height = cinfo.output_height;
  int channel = cinfo.output_components;

  std::vector<unsigned char> tempData(_width * _height * channel);
  while (cinfo.output_scanline < _height)
  {
    unsigned char *rowPointer = tempData.data() + cinfo.output_scanline * _width * channel;
    jpeg_read_scanlines(&cinfo, &rowPointer, 1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(jpegFile);

  _imageData.resize(_width * _height * 4);
  for (int y = 0; y < _height; y++)
  {
    for (int x = 0; x < _width; x++)
    {
      int tempIndex = (y * _width + x) * channel;
      int imgIndex = (y * _width + x) * 4;

      _imageData[imgIndex] = tempData[tempIndex];
      _imageData[imgIndex + 1] = tempData[tempIndex + 1];
      _imageData[imgIndex + 2] = tempData[tempIndex + 2];
      _imageData[imgIndex + 3] = 255;
    }
  }

  return true;
}

void BackgroundManager::DrawBackground(bool isRedraw, BackgroundType type)
{
  if (_needReload && !isRedraw)
  {
    SetImageFile(_imageFile);

    if (_pixmap != 0)
    {
      xcb_free_pixmap(_connection, _pixmap);
      _pixmap = 0;
    }

    if (_gc != 0)
    {
      xcb_free_gc(_connection, _gc);
      _gc = 0;
    }
  }

  if (_imageData.empty())
  {
    std::cout << "[Error]: " << "image data is empty maybe not loaded" << std::endl;
    return;
  }

  xcb_image_t *image = xcb_image_create_native(_connection, _width, _height, XCB_IMAGE_FORMAT_Z_PIXMAP, _depth, nullptr, ~0, _imageData.data());

  xcb_void_cookie_t cookie;
  xcb_generic_error_t *error;

  if (!isRedraw && _pixmap == 0)
  {
    _pixmap = xcb_generate_id(_connection);
    cookie = xcb_create_pixmap_checked(_connection, _depth, _pixmap, _window, _width, _height);
    error = xcb_request_check(_connection, cookie);
  }

  if (error)
  {
    std::cout << "[Error]: " << "pixmap won't created with error code " << (int)error->error_code << std::endl;
    return;
  }

  if (!isRedraw && _gc == 0)
  {
    _gc = xcb_generate_id(_connection);
    cookie = xcb_create_gc_checked(_connection, _gc, _pixmap, 0, nullptr);
    error = xcb_request_check(_connection, cookie);
  }

  if (error)
  {
    std::cout << "[Error]: " << "gc won't created with error code " << (int)error->error_code << std::endl;
    return;
  }

  cookie = xcb_image_put(_connection, _pixmap, _gc, image, 0, 0, 0);
  error = xcb_request_check(_connection, cookie);

  if (error)
  {
    std::cout << "[Error]: " << "image won't put to pixmap with error code " << (int)error->error_code << std::endl;
    return;
  }

  xcb_configure_window(_connection, _window, XCB_CW_BACK_PIXMAP, &_pixmap);
  xcb_clear_area(_connection, 0, _window, 0, 0, _winWidth, _winHeight);
  xcb_flush(_connection);

  int xOffset = (_winWidth - _width) / 2;
  int yOffset = (_winHeight - _height) / 2;

  xcb_copy_area(_connection, _pixmap, _window, _gc, 0, 0, xOffset, yOffset, _width, _height);
  xcb_flush(_connection);
}

void BackgroundManager::SetWindow(xcb_window_t win)
{
  _window = win;

  xcb_generic_error_t *error = nullptr;

  xcb_get_geometry_cookie_t geometryCookie = xcb_get_geometry_unchecked(_connection, _window);
  xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(_connection, geometryCookie, &error);

  _winWidth = geometry->width;
  _winHeight = geometry->height;

  free(geometry);
}

void BackgroundManager::SetImageFile(std::string imageFile)
{
  _imageFile = imageFile;

  if (_imageFile.find(".png") != std::string::npos)
  {
    if (!LoadPng())
    {
      std::cout << "[Error]: " << "Could not load image maybe this is not png or wrong path" << std::endl;
      return;
    }
  }
  else if (_imageFile.find(".jpeg") != std::string::npos || _imageFile.find(".jpg") != std::string::npos)
  {
    if (!LoadJpeg())
    {
      std::cout << "[Error]: " << "Could not load image maybe this is not jpe/g or wrong path" << std::endl;
      return;
    }
  }

  if (_width > _winWidth || _height > _winHeight)
  {
    ResizeImage();
  }
}

void BackgroundManager::ScaledImage()
{
}

void BackgroundManager::ResizeImage()
{
  float widthRatio = static_cast<float>(_winWidth) / _width;
  float heightRatio = static_cast<float>(_winHeight) / _height;
  float scale = std::min(widthRatio, heightRatio);

  // new dimention
  int newWidth = static_cast<int>(_width * scale);
  int newHeight = static_cast<int>(_height * scale);

  std::vector<unsigned char> resizedImage(newWidth * newHeight * 4);
  for (int y = 0; y < newHeight; y++)
  {
    for (int x = 0; x < newWidth; x++)
    {
      int srcX = static_cast<int>(x / scale);
      int srcY = static_cast<int>(y / scale);
      int srcIndex = (srcY * _width + srcX) * 4;
      int dstIndex = (y * newWidth + x) * 4;

      resizedImage[dstIndex] = _imageData[srcIndex + 2];
      resizedImage[dstIndex + 1] = _imageData[srcIndex + 1];
      resizedImage[dstIndex + 2] = _imageData[srcIndex];
      resizedImage[dstIndex + 3] = _imageData[srcIndex + 3];
    }
  }

  _imageData = std::move(resizedImage);
  _width = newWidth;
  _height = newHeight;
  _needReload = true;
}