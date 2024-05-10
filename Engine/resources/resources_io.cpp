#define NOMINMAX  // Required for assimp

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


#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/LogStream.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "engine/log.h"
#include "engine/io.h"
#include "math/vec3.h"
#include "math/math.h"

#include "assets/mesh.h"
#include "assets/texture.h"
#include "renderer/dx11_lib.h"


namespace engine
{
  const unsigned int assimp_import_flags = 
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_SortByPType |
    aiProcess_PreTransformVertices |
    aiProcess_GenNormals |
    aiProcess_GenUVCoords |
    aiProcess_OptimizeMeshes |
    aiProcess_Debone |
    aiProcess_ValidateDataStructure |
    aiProcess_GenBoundingBoxes;

  struct assimp_logger : public Assimp::LogStream
  {
    static void initialize()
    {
      if(Assimp::DefaultLogger::isNullLogger()) {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
        Assimp::DefaultLogger::get()->attachStream(new assimp_logger, Assimp::Logger::Err | Assimp::Logger::Warn);
      }
    }
	
    void write(const char* message) override
    {
      LOG_ERROR("Assimp: {0}", message);
    }
  };

  bool load_obj(const std::string& file_name, astatic_mesh* out_static_mesh)
  {
    assert(out_static_mesh);
    
    std::string path = fio::get_mesh_file_path(file_name.c_str());

    // Parse OBJ file
    Assimp::Importer importer;
    assimp_logger::initialize();

    const aiScene* ai_scene = importer.ReadFile(path, assimp_import_flags);
    if(ai_scene && ai_scene->HasMeshes())
    {
      const aiMesh* ai_mesh = ai_scene->mMeshes[0];
      assert(ai_mesh->HasPositions());
      assert(ai_mesh->HasNormals());

      // Vertex list
      {
        std::vector<fvertex_data> vertex_list;
        vertex_list.reserve(ai_mesh->mNumVertices);
        for(size_t i=0; i<vertex_list.capacity(); ++i)
        {
          fvertex_data vertex;
          vertex.position = { ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z };
          vertex.normal   = { ai_mesh->mNormals[i].x,  ai_mesh->mNormals[i].y,  ai_mesh->mNormals[i].z };
          if(ai_mesh->HasTangentsAndBitangents())
          {
            vertex.tangent    = { ai_mesh->mTangents[i].x,   ai_mesh->mTangents[i].y,   ai_mesh->mTangents[i].z };
            vertex.bitangent  = { ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z };
          }
          if(ai_mesh->HasTextureCoords(0))
          {
            vertex.uv = { ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y };
          }
          vertex_list.push_back(vertex);
        }
        // Vertex buffer
        fdx11::instance().create_vertex_buffer(vertex_list, out_static_mesh->render_state.vertex_buffer);
        
        out_static_mesh->render_state.offset = 0;
        out_static_mesh->render_state.stride = sizeof(fvertex_data);
      }

      // Face list
      {
        std::vector<fface_data> face_list;
        face_list.reserve(ai_mesh->mNumFaces);
        for(size_t i=0; i<face_list.capacity(); ++i)
        {
          assert(ai_mesh->mFaces[i].mNumIndices == 3);
          const aiFace& ai_face = ai_mesh->mFaces[i];
          face_list.push_back(std::move(fface_data(ai_face.mIndices[0], ai_face.mIndices[1], ai_face.mIndices[2])));
        }
        // Index buffer
        fdx11::instance().create_index_buffer(face_list, out_static_mesh->render_state.index_buffer);
        out_static_mesh->render_state.num_indices = static_cast<int32_t>(face_list.size()) * 3;
      }
      
      // Bounding box
      const aiAABB& box = ai_mesh->mAABB;
      const aiVector3D center = (box.mMax + box.mMin) * 0.5f;
      const aiVector3D extent = (box.mMin - box.mMax) * 0.5f;
      out_static_mesh->bounding_box.Center = { center.x, center.y, center.z };
      out_static_mesh->bounding_box.Extents = { extent.x, extent.y, extent.z };
    }
    else
    {
        LOG_WARN("Failed to open object file: {0}", file_name);
        return false;
    }
    return true;
  }
  
  bool load_img(const std::string& file_name, int desired_channels, atexture* out_texture)
  {
    assert(out_texture);
    
    std::string path = fio::get_texture_file_path(file_name.c_str());
    
    int num_channels = 0;
    bool is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));
    uint8_t* data_ldr = nullptr;
    float* data_hdr = nullptr;
    int bytes_per_row = 0;
    const void* buffer = nullptr;
    
    if (is_hdr)
    {
      data_hdr = stbi_loadf(path.c_str(), &out_texture->width, &out_texture->height, &num_channels, desired_channels);
      if (data_hdr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
      buffer = static_cast<const void*>(data_hdr);
      bytes_per_row = out_texture->desired_channels * sizeof(float) * out_texture->width;
    }
    else
    {
      data_ldr = stbi_load(path.c_str(), &out_texture->width, &out_texture->height, &num_channels, desired_channels);
      if (data_ldr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
      buffer = static_cast<const void*>(data_ldr);
      bytes_per_row = out_texture->desired_channels * sizeof(uint8_t) * out_texture->width;
    }

    fdx11::instance().create_shader_resource_view(out_texture->width, out_texture->height, is_hdr, bytes_per_row, buffer, out_texture->render_state.texture_srv);
    delete buffer;
    return true;
  }
  
  bool load_hlsl(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<ID3D10Blob>& out_shader_blob)
  {
    std::string path = fio::get_shader_file_path(file_name.c_str());
    
    ComPtr<ID3DBlob> shader_compiler_errors_blob;
    std::wstring wpath = std::wstring(path.begin(), path.end());
    LPCWSTR sw = wpath.c_str();
    HRESULT result = D3DCompileFromFile(sw, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), 0, 0, out_shader_blob.GetAddressOf(), shader_compiler_errors_blob.GetAddressOf());
    if (FAILED(result))
    {
      if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
      {
        LOG_ERROR("Could not compile shader, file {0} not found.", file_name);
      }
      else if (shader_compiler_errors_blob)
      {
        LOG_ERROR("Could not copile shader. {0}", static_cast<const char*>(shader_compiler_errors_blob->GetBufferPointer()));
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

