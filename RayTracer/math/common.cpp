#include "stdafx.h"

#include <filesystem>
#include <random>
#include <fstream>

#include "gfx/stb_image.h"
#include "gfx/tiny_obj_loader.h"

#include "common.h"






namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces)
  {
    assert(shape_index >= 0);

    tinyobj::attrib_t attributes; // not implemented
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; // not implemented

    std::string dir = engine::io::get_meshes_dir();
    std::string path = engine::io::get_mesh_file_path(file_name.c_str());

    std::string error;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.c_str(), dir.c_str(), true))
    {
      LOG_ERROR("Unable to load object file: {0} {1}", path, error);
      return false;
    }
    if (shape_index >= shapes.size())
    {
      LOG_ERROR("Object file: {0} does not have shape index: {1}", path, shape_index);
      return false;
    }

    tinyobj::shape_t shape = shapes[shape_index];
    size_t num_faces = shape.mesh.num_face_vertices.size();
    if (num_faces == 0)
    {
      LOG_ERROR("Object file: {0} has no faces", path);
      return false;
    }
    out_faces.reserve(num_faces);

    // loop over faces
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
      out_faces.push_back(triangle_face());
      triangle_face& face = out_faces[fi];

      // loop over the vertices in the face
      assert(shape.mesh.num_face_vertices[fi] == 3);
      for (size_t vi = 0; vi < 3; ++vi)
      {
        tinyobj::index_t idx = shape.mesh.indices[3 * fi + vi];

        if (idx.vertex_index == -1)
        {
          LOG_ERROR("Object file: {0} faces not found", file_name);
          return false;
        }
        if (idx.normal_index == -1)
        {
          LOG_ERROR("Object file: {0} normals not found", file_name);
          return false;
        }
        if (idx.texcoord_index == -1)
        {
          LOG_ERROR("Object file: {0} UVs not found", file_name);
          return false;
        }

        float vx = attributes.vertices[3 * idx.vertex_index + 0];
        float vy = attributes.vertices[3 * idx.vertex_index + 1];
        float vz = attributes.vertices[3 * idx.vertex_index + 2];
        face.vertices[vi] = vec3(vx, vy, vz);

        float nx = attributes.normals[3 * idx.normal_index + 1];
        float ny = attributes.normals[3 * idx.normal_index + 2];
        float nz = attributes.normals[3 * idx.normal_index + 0];
        face.normals[vi] = math::normalize(vec3(nx, ny, nz));
        
        float uvx = attributes.texcoords[2 * idx.texcoord_index + 0];
        float uvy = attributes.texcoords[2 * idx.texcoord_index + 1];
        face.UVs[vi] = vec3(uvx, uvy, 0.0f);
      }
    }
    
    return true;
  }
}

namespace img_helper
{
  bool load_img(const std::string& file_name, int width, int height, engine::texture* out_texture)
  {
    std::string path = engine::io::get_texture_file_path(file_name.c_str());

    out_texture->is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));

    if (out_texture->is_hdr)
    {
      out_texture->data_hdr = stbi_loadf(path.c_str(), &width, &height, nullptr, STBI_rgb);
      if (out_texture->data_hdr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
    }
    else
    {
      out_texture->data_ldr = stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb);
      if (out_texture->data_ldr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
    }
    return true;
  }
}

