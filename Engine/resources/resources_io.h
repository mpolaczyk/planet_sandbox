#pragma once

#include "core/core.h"

#include <vector>
#include <string>

#include "math/triangle_face.h"
#include "assets/texture.h"

namespace engine
{
  bool ENGINE_API load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces);

  bool ENGINE_API load_img(const std::string& file_name, int width, int height, texture_asset* out_texture);
}

