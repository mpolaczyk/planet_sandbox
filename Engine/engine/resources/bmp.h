#pragma once

#include <stdint.h>

#include "core/core.h"
#include "engine/math/math.h"
#include "engine/math/vec3.h"

namespace engine
{
  constexpr uint32_t BYTES_PER_PIXEL = 4; /// RGBA (A not in use, only for storage)
  constexpr uint32_t FILE_HEADER_SIZE = 14;
  constexpr uint32_t INFO_HEADER_SIZE = 40;

  enum class bmp_format
  {
    rgba = 0,
    // DXGI_FORMAT_R8G8B8A8
    bgra // BMP file
  };

  struct ENGINE_API fbmp_pixel
  {
    uint8_t b = 0;
    uint8_t g = 0;
    uint8_t r = 0;
    uint8_t a = 255;
    fbmp_pixel() = default;

    fbmp_pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b)
    {
    }

    explicit fbmp_pixel(const fvec3& color);
  };


  struct ENGINE_API fbmp_image
  {
    fbmp_image(uint32_t w, uint32_t h);
    ~fbmp_image();

    void draw_pixel(uint32_t x, uint32_t y, const fbmp_pixel* p, bmp_format format = bmp_format::bgra);
    void save_to_file(const char* image_file_name) const;
    uint8_t* get_buffer() const { return buffer; }
    uint32_t get_width() const { return width; }
    uint32_t get_height() const { return height; }

  private:
    uint8_t* create_file_header(uint32_t height, uint32_t stride) const;
    uint8_t* create_info_header(uint32_t height, uint32_t width) const;

    uint8_t* buffer = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
  };
};
