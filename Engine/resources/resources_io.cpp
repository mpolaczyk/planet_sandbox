
#include <filesystem>
#include <random>
#include <fstream>
#include <cassert>

#include "resources/resources_io.h"

// FIX This cpp file is getting really heavy, split it

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include "engine/log.h"
#include "engine/io.h"
#include "math/vec3.h"
#include "math/math.h"

#include "assets/mesh.h"
#include "assets/texture.h"

#include "third_party/WaveFrontReader.h"

namespace engine
{
  bool load_obj(const std::string& file_name, static_mesh_asset* out_static_mesh)
  {
    assert(out_static_mesh);
    
    std::string dir = io::get_meshes_dir();
    std::string path = io::get_mesh_file_path(file_name.c_str());

    WaveFrontReader<uint16_t> obj_reader;
    {
      std::wstring widestr = std::wstring(path.begin(), path.end());
      const wchar_t* widecstr = widestr.c_str();
      HRESULT result = obj_reader.Load(widecstr);
      if(FAILED(result))
      {
        LOG_ERROR("Unable to load object file: {0}", path);
        return false;
      }
    }
    if(!obj_reader.hasNormals)
    {
      LOG_WARN("Static mesh: {0} has no normals!", file_name);
    }
    if(!obj_reader.hasTexcoords)
    {
      LOG_WARN("Static mesh: {0} has no texture coords!", file_name); 
    }
    //out_static_mesh->materials = obj_reader.materials;   // FIX not implemented yet
    out_static_mesh->bounding_box = obj_reader.bounds;
    out_static_mesh->index_buffer = obj_reader.indices;
    for (auto& vert : obj_reader.vertices)
    {
      vertex_data vd;
      vd.pos = vert.position;
      vd.norm = vert.normal;
      vd.uv = vert.textureCoordinate;
      out_static_mesh->vertex_buffer.push_back(vd);
    }
    for(int i = 0; i < obj_reader.indices.size()-3; i+=3)
    {
      triangle_face tf;
      for(int j = 0; j < 3; j++)
      {
        tf.vertices[j].x = obj_reader.vertices[obj_reader.indices[i+j]].position.x;
        tf.vertices[j].y = obj_reader.vertices[obj_reader.indices[i+j]].position.y;
        tf.vertices[j].z = obj_reader.vertices[obj_reader.indices[i+j]].position.z;
        tf.normals[j].x = obj_reader.vertices[obj_reader.indices[i+j]].normal.x;
        tf.normals[j].y = obj_reader.vertices[obj_reader.indices[i+j]].normal.y;
        tf.normals[j].z = obj_reader.vertices[obj_reader.indices[i+j]].normal.z;
        tf.UVs[j].x = obj_reader.vertices[obj_reader.indices[i+j]].textureCoordinate.x;
        tf.UVs[j].y = obj_reader.vertices[obj_reader.indices[i+j]].textureCoordinate.y;
      }
      out_static_mesh->faces.push_back(tf);
    }
    return true;
  }

  bool load_img(const std::string& file_name, int desired_channels, texture_asset* out_texture)
  {
    assert(out_texture);
    
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

