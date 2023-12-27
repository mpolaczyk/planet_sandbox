
#include <filesystem>
#include <random>
#include <fstream>
#include <cassert>

#include "resources/resources_io.h"

// FIX This cpp file is getting really heavy, split it

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "third_party/tiny_obj_loader.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include "engine/log.h"
#include "engine/io.h"
#include "math/vec3.h"
#include "math/math.h"

namespace engine
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces)
  {
    assert(shape_index >= 0);

    tinyobj::attrib_t attributes; // not implemented
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; // not implemented

    std::string dir = io::get_meshes_dir();
    std::string path = io::get_mesh_file_path(file_name.c_str());

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

  bool load_img(const std::string& file_name, int desired_channels, texture_asset* out_texture)
  {
    std::string path = io::get_texture_file_path(file_name.c_str());

    out_texture->is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));
    
    if (out_texture->is_hdr)
    {
      out_texture->data_hdr = stbi_loadf(path.c_str(), &out_texture->width, &out_texture->height, &out_texture->num_channels, desired_channels);
      if (out_texture->data_hdr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
    }
    else
    {
      out_texture->data_ldr = stbi_load(path.c_str(), &out_texture->width, &out_texture->height, &out_texture->num_channels, desired_channels);
      if (out_texture->data_ldr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
    }
    return true;
  }
  
  bool load_hlsl(const std::string& file_name, const std::string& entrypoint, const std::string& target, ID3D10Blob** out_shader_blob)
  {
    std::string path = io::get_shader_file_path(file_name.c_str());
    
    ID3DBlob* shader_compiler_errors_blob = nullptr;
    std::wstring wpath = std::wstring(path.begin(), path.end());
    LPCWSTR sw = wpath.c_str();
    HRESULT result = D3DCompileFromFile(sw, nullptr, nullptr, entrypoint.c_str(), target.c_str(), 0, 0, out_shader_blob, &shader_compiler_errors_blob);
    if (FAILED(result))
    {
      if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
      {
        LOG_ERROR("Could not compile shader, file {0} not found.", file_name);
      }
      else if (shader_compiler_errors_blob)
      {
        LOG_ERROR("Could not copile shader. {0}", static_cast<const char*>(shader_compiler_errors_blob->GetBufferPointer()));
        shader_compiler_errors_blob->Release();
      }
      else
      {
        LOG_ERROR("Could not copile shader. Result: {0}", result);
      }
      return false;
    }
    return true;
  }
}

