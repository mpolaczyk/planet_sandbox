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
//#include <d3dcompiler.h>  // fxc
#include "dxcapi.h"         // dxc

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
#include "engine/string_tools.h"
#include "renderer/dx12_lib.h"


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
      if(!ai_mesh->HasPositions() || !ai_mesh->HasNormals() || ai_mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
      {
        LOG_WARN("Failed to parse object file: {0}", file_name);
        return false;
      }
      // Vertex list
      {
        std::vector<fvertex_data>& vertex_list = out_static_mesh->render_state.vertex_list;
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
        std::vector<fface_data>& face_list = out_static_mesh->render_state.face_list;
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
      const aiVector3D center = (box.mMax + box.mMin) * 0.5f;
      const aiVector3D extent = (box.mMin - box.mMax) * 0.5f;
      out_static_mesh->bounding_box.Center = {center.x, center.y, center.z};
      out_static_mesh->bounding_box.Extents = {extent.x, extent.y, extent.z};
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

    out_texture->render_state.is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));
    
    if(out_texture->render_state.is_hdr)
    {
      float* data_hdr = stbi_loadf(path.c_str(), &out_texture->width, &out_texture->height, &out_texture->channels, 4);
      int32_t elements_total = out_texture->channels * out_texture->width * out_texture->height;
      if(data_hdr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
      out_texture->render_state.data_hdr.resize(elements_total, 0.0f);
      std::vector<float> z(data_hdr, data_hdr + elements_total);
      out_texture->render_state.data_hdr = z;

    }
    else
    {
      uint8_t* data_ldr = stbi_load(path.c_str(), &out_texture->width, &out_texture->height, &out_texture->channels, 4);
      int32_t elements_total = out_texture->channels * out_texture->width * out_texture->height;
      if(data_ldr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
      out_texture->render_state.data_ldr.resize(elements_total, 0);
      std::vector<uint8_t> z(data_ldr, data_ldr + elements_total);
      out_texture->render_state.data_ldr = z;
    }
    return true;
  }

  // https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll
  // https://simoncoenen.com/blog/programming/graphics/DxcCompiling
  bool load_hlsl_dxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<IDxcBlob>& out_shader_blob)
  {
    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcIncludeHandler> include_handler;
    const std::string shader_directory = fio::get_shaders_dir();
    const std::string hlsl_path = fio::get_shader_file_path(file_name.c_str());
    
    std::vector<LPCWSTR> arguments;
    std::wstring w_file_name = fstring_tools::to_utf16(file_name);
    std::wstring w_hlsl_path = fstring_tools::to_utf16(hlsl_path);
    std::wstring w_entrypoint = fstring_tools::to_utf16(entrypoint);
    std::wstring w_target = fstring_tools::to_utf16(target);
    std::wstring w_shader_directory = fstring_tools::to_utf16(shader_directory);
    arguments.push_back(w_file_name.c_str());
    arguments.push_back(L"-E");
    arguments.push_back(w_entrypoint.c_str());
    arguments.push_back(L"-T");
    arguments.push_back(w_target.c_str());
    arguments.push_back(L"-I");
    arguments.push_back(w_shader_directory.c_str());
#if BUILD_DEBUG
    arguments.push_back(L"-Od");            // Disable optimizations
    arguments.push_back(L"-Zi");            // Enable debug information
    arguments.push_back(L"-Zss");           // Create hash using source information
#endif
    
    // Initialize the dxc
    // TODO: compiler and utils does not have to be created for each invocation
    if(FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))))
    {
      LOG_ERROR("Could not compile shader, failed to create dxc utils instance.");
      return false;
    }
    if(FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
    {
      LOG_ERROR("Could not compile shader, failed to create dxc instance.");
      return false;
    }
    if(FAILED(utils->CreateDefaultIncludeHandler(&include_handler)))
    {
      LOG_ERROR("Could not compile shader, failed to create dxc include handler.");
      return false;
    }

    // Load shader source
    ComPtr<IDxcBlobEncoding> source_blob;
    if(FAILED(utils->LoadFile(w_hlsl_path.c_str(), nullptr, source_blob.GetAddressOf())))
    {
      LOG_ERROR("Could not compile shader, failed to load file.");
      return false;
    }

    // Compile shader source
    DxcBuffer source;   
    source.Ptr = source_blob->GetBufferPointer();
    source.Size = source_blob->GetBufferSize();
    source.Encoding = DXC_CP_ACP;
    ComPtr<IDxcResult> dxc_result;
    if(FAILED(compiler->Compile(&source, arguments.data(), arguments.size(), include_handler.Get(), IID_PPV_ARGS(dxc_result.GetAddressOf()))))
    {
      LOG_ERROR("Could not compile shader, failed to create dxc utils instance.");
      return false;
    }

    // Print warnings and errors, fail if errors
    ComPtr<IDxcBlobUtf8> errors = nullptr;
    fdx12::get_dxc_blob(dxc_result, DXC_OUT_ERRORS, errors);
    if (errors != nullptr && errors->GetStringLength() != 0)
    {
      LOG_ERROR("Warnings and errors:\n {0}", errors->GetStringPointer());
    }
    HRESULT hr;
    dxc_result->GetStatus(&hr);
    if (FAILED(hr))
    {
      LOG_ERROR("Could not compile shader, compilation failed.");
      return false;
    }
    
    // Get object file
    if(!fdx12::get_dxc_blob(dxc_result, DXC_OUT_OBJECT, out_shader_blob))
    {
      return false;
    }
    
#if BUILD_DEBUG

    // Find shader hash
    ComPtr<IDxcBlob> hash = nullptr;
    std::string hash_str;
    char hash_string[32] = {'\0'};
    if(fdx12::get_dxc_blob(dxc_result, DXC_OUT_SHADER_HASH, hash) && hash != nullptr)
    {
      auto* pHashBuf = static_cast<DxcShaderHash*>(hash->GetBufferPointer());
      
      for(size_t i = 0; i < _countof(pHashBuf->HashDigest); ++i)
      {
        snprintf(hash_string + i, 16, "%X", pHashBuf->HashDigest[i]);
      }
      hash_str = std::string(hash_string);
      LOG_INFO("Hash: {0}", hash_str);
    }
    
    // Save object file
    std::string obj_path = fio::get_shader_file_path(hash_string) + ".bin";
    if(!fdx12::save_dxc_blob(out_shader_blob, obj_path.c_str()))
    {
      return false;
    }
    
    // Get debug symbols
    ComPtr<IDxcBlob> pdb_blob;
    if(!fdx12::get_dxc_blob(dxc_result, DXC_OUT_PDB, pdb_blob))
    {
      return false;
    }

    // Save debug symbols
    std::string pdb_path = fio::get_shader_file_path(hash_string) + ".pdb";
    if(!fdx12::save_dxc_blob(pdb_blob, pdb_path.c_str()))
    {
      return false;
    }
#endif
    
    return true;
  }

  // https://asawicki.info/news_1719_two_shader_compilers_of_direct3d_12
  // https://github.com/NVIDIAGameWorks/ShaderMake/blob/470bbc7d0c343bc82c988072ee8a1fb2210647ce/src/ShaderMake.cpp#L988
  // https://devblogs.microsoft.com/pix/using-automatic-shader-pdb-resolution-in-pix/
//  bool load_hlsl_fxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, ComPtr<ID3D10Blob>& out_shader_blob)
//  {
//    std::string hlsl_path = fio::get_shader_file_path(file_name.c_str());
//
//    ComPtr<ID3DBlob> shader_compiler_errors_blob;
//    UINT flags1 = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
//#if BUILD_DEBUG
//    flags1 |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG_NAME_FOR_BINARY;
//#elif BUILD_RELEASE
//    flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS;
//#endif
//    HRESULT result = D3DCompileFromFile(std::wstring(hlsl_path.begin(), hlsl_path.end()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
//      entrypoint.c_str(), target.c_str(), flags1, 0, out_shader_blob.GetAddressOf(), shader_compiler_errors_blob.GetAddressOf());
//    if(FAILED(result))
//    {
//      if(result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
//      {
//        LOG_ERROR("Could not compile shader, file {0} not found.", file_name);
//      }
//      else if(shader_compiler_errors_blob)
//      {
//        LOG_ERROR("Could not compile shader. {0}", static_cast<const char*>(shader_compiler_errors_blob->GetBufferPointer()));
//      }
//      else
//      {
//        LOG_ERROR("Could not compile shader. Result: {0}", result);
//      }
//      return false;
//    }
//    
//    const char* pdb_file_name = nullptr;
//#if BUILD_DEBUG
//    // Save the PDB file
//    {
//      // Retrieve the debug info part of the shader
//      ComPtr<ID3DBlob> pdb;
//      D3DGetBlobPart(out_shader_blob->GetBufferPointer(), out_shader_blob->GetBufferSize(), D3D_BLOB_PDB, 0, &pdb);
//      
//      // Retrieve the suggested name for the debug data file
//      ComPtr<ID3DBlob> pdb_name;
//      D3DGetBlobPart(out_shader_blob->GetBufferPointer(), out_shader_blob->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, &pdb_name);
//      
//      // This struct represents the first four bytes of the name blob
//      struct shader_debug_name
//      {
//        uint16_t flags;       // Reserved, must be set to zero
//        uint16_t name_length;  // Length of the debug name, without null terminator
//        // Followed by name_length bytes of the UTF-8-encoded name
//        // Followed by a null terminator
//        // Followed by [0-3] zero bytes to align to a 4-byte boundary
//      };
//      
//      auto debug_name_data = (const shader_debug_name*)(pdb_name->GetBufferPointer());
//      pdb_file_name = (const char*)(debug_name_data + 1);
//    
//      std::string file = fio::get_shader_file_path(pdb_file_name);
//      FILE* fp = fopen(file.c_str(), "wb");
//      if (fp)
//      {
//        fwrite(pdb->GetBufferPointer(), pdb->GetBufferSize(), 1, fp);
//        fclose(fp);
//      }
//    }
//#elif BUILD_RELEASE
//    // Strip reflection
//    ComPtr<ID3DBlob> stripped_blob;
//    UINT flags = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA;
//    D3DStripShader(out_shader_blob->GetBufferPointer(), out_shader_blob->GetBufferSize(), flags, &stripped_blob);
//    out_shader_blob = stripped_blob;
//#endif
//
//    // Save CSO to the disk
//    {
//      if(pdb_file_name)
//      {
//        std::string file = fio::get_shader_file_path(pdb_file_name);
//        std::size_t index = file.find_last_of(".");
//        file.replace(index, 4, ".cso");
//        std::wstring cso_file = std::wstring(file.begin(), file.end());
//        D3DWriteBlobToFile(out_shader_blob.Get(), std::wstring(cso_file.begin(), cso_file.end()).c_str(), true);
//      }
//    }
//    return true;
//  }
  
}
