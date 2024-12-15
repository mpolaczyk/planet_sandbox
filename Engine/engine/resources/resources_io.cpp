#define NOMINMAX  // Required for assimp

#include <filesystem>
#include <random>
#include <fstream>
#include <cassert>

#include "engine/resources/resources_io.h"

// FIX This cpp file is getting really heavy, split it

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/LogStream.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "engine/log.h"
#include "engine/io.h"
#include "engine/math/vec3.h"
#include "engine/math/math.h"

#include "assets/mesh.h"
#include "assets/texture.h"
#include "core/application.h"

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
      if(Assimp::DefaultLogger::isNullLogger())
      {
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
      if(!ai_mesh->HasPositions() || !ai_mesh->HasNormals())// || ai_mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
      {
        LOG_WARN("Failed to parse object file: {0}", file_name);
        return false;
      }
      // Vertex list
      // TODO Waste! Vertices are repeated for each triangle they are in (triangle list)!
      {
        std::vector<fvertex_data>& vertex_list = out_static_mesh->vertex_list;
        vertex_list.resize(ai_mesh->mNumVertices, fvertex_data());
        for(size_t i = 0; i < vertex_list.capacity(); ++i)
        {
          fvertex_data& vertex = vertex_list[i];
          vertex.position = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z};
          vertex.normal = {ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z};
          if(ai_mesh->HasTangentsAndBitangents())
          {
            vertex.tangent = {ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z};
            vertex.bitangent = {ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z};
          }
          if(ai_mesh->HasTextureCoords(0))
          {
            vertex.uv = {ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};
          }
        }
      }

      // Face list
      {
        std::vector<fface_data>& face_list = out_static_mesh->face_list;
        face_list.resize(ai_mesh->mNumFaces, fface_data());
        for(size_t i = 0; i < face_list.capacity(); ++i)
        {
          assert(ai_mesh->mFaces[i].mNumIndices == 3);

          fface_data& face = face_list[i];
          face.v1 = ai_mesh->mFaces[i].mIndices[0];
          face.v2 = ai_mesh->mFaces[i].mIndices[1];
          face.v3 = ai_mesh->mFaces[i].mIndices[2];
        }
      }

      // Bounding box
      const aiAABB& box = ai_mesh->mAABB;
      const fvec3 min(box.mMin.x, box.mMin.y, box.mMin.z);
      const fvec3 max(box.mMax.x, box.mMax.y, box.mMax.z);
      out_static_mesh->bounding_box = fbounding_box::from_min_max(min, max);
    }
    else
    {
      LOG_WARN("Failed to open object file: {0}", file_name);
      return false;
    }
    return true;
  }

  bool load_img(const std::string& file_name, atexture* out_texture)
  {
    assert(out_texture);

    std::string path = fio::get_texture_file_path(file_name.c_str());

    bool is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));
    
    int width = 0;
    int height = 0;
    int channels = 0;
    
    if(is_hdr)
    {
      float* data_hdr = stbi_loadf(path.c_str(), &width, &height, &channels, 4);
      int32_t elements_total = channels * width * height;
      if(data_hdr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }

      out_texture->data_hdr.resize(elements_total, 0.0f);
      std::vector<float> z(data_hdr, data_hdr + elements_total);
      out_texture->data_hdr = z;
      out_texture->element_size = sizeof(float);
      out_texture->format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
    else
    {
      uint8_t* data_ldr = stbi_load(path.c_str(), &width, &height, &channels, 4);
      int32_t elements_total = channels * width * height;
      if(data_ldr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
      out_texture->data_ldr.resize(elements_total, 0);
      std::vector<uint8_t> z(data_ldr, data_ldr + elements_total);
      out_texture->data_ldr = z;
      out_texture->element_size = sizeof(uint8_t);
      out_texture->format = DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    out_texture->is_hdr = is_hdr;
    out_texture->width = width;
    out_texture->height = height;
    out_texture->channels = channels;
    return true;
  }

}
