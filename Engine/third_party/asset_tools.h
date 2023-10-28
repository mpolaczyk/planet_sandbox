#pragma once

#include "core/core.h"

#include <vector>
#include <string>

#include "math/triangle_face.h"
#include "asset/textures.h"

namespace engine
{
  bool ENGINE_API load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces);

  bool ENGINE_API load_img(const std::string& file_name, int width, int height, texture* out_texture);
}

