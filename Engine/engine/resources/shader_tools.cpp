#include "stdafx.h"

#include <cassert>
#include <dbghelp.h>

#pragma comment(lib, "dxcompiler.lib")

#if USE_FXC
#include <d3dcompiler.h>
#endif

#include "engine/resources/shader_tools.h"
#include "engine/io.h"
#include "engine/string_tools.h"
#include "engine/math/math.h"

namespace engine
{
  namespace
  {
    bool get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, fcom_ptr<IDxcBlob>& out_blob)
    {
      fcom_ptr<IDxcBlobUtf16> name = nullptr;
      if(FAILED(result->GetOutput(blob_type, IID_PPV_ARGS(out_blob.GetAddressOf()), name.GetAddressOf())) && out_blob.Get() != nullptr)
      {
        LOG_ERROR("Unable to get dxc blob {0}", static_cast<int32_t>(blob_type));
        return false;
      }
      return true;
    }
  
    bool get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, fcom_ptr<IDxcBlobUtf8>& out_blob)
    {
      fcom_ptr<IDxcBlobUtf16> name = nullptr;
      if(FAILED(result->GetOutput(blob_type, IID_PPV_ARGS(out_blob.GetAddressOf()), name.GetAddressOf())) && out_blob.Get() != nullptr)
      {
        LOG_ERROR("Unable to get dxc blob {0}", static_cast<int32_t>(blob_type))
        return false;
      }
      return true;
    } 

    bool get_dxc_hash(IDxcResult* result, std::string& out_hash)
    {
      fcom_ptr<IDxcBlob> hash = nullptr;
      char hash_string[32] = {'\0'};
      if(get_dxc_blob(result, DXC_OUT_SHADER_HASH, hash) && hash.Get() != nullptr)
      {
        auto* hash_buffer = static_cast<DxcShaderHash*>(hash->GetBufferPointer());
        for(size_t i = 0; i < _countof(hash_buffer->HashDigest); ++i)
        {
          snprintf(hash_string + i, 16, "%X", hash_buffer->HashDigest[i]);
        }
        out_hash = std::string(hash_string);
        return true;
      }
      return false;
    }
    
    bool save_dxc_blob(IDxcBlob* blob, const char* path)
    {
      FILE* file = nullptr;
      std::wstring w_path = fstring_tools::to_utf16(path);
      errno_t open_result = _wfopen_s(&file, w_path.c_str(), L"wb");
      if(open_result != 0)
      {
        // See https://learn.microsoft.com/en-us/cpp/c-runtime-library/errno-constants
        LOG_ERROR("Failed to write dxc blob. Error code {0}", static_cast<int32_t>(open_result))
        return false;
      }
      fwrite(blob->GetBufferPointer(), blob->GetBufferSize(), 1, file);
      fclose(file);
      return true;
    }
  }
  
  bool fshader_tools::load_compiled_shader(const std::string& name, fcom_ptr<IDxcBlob>& out_shader_blob)
  {
#if FORCE_COMPILE_SHADERS_ON_START
    return false;
#endif
    fcom_ptr<IDxcUtils> utils;
    if(FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()))))
    {
      LOG_ERROR("Failed to create dxc utils instance.");
      return false;
    }

    // Load shader object
    {
      fcom_ptr<IDxcBlobEncoding> cso_blob;
      const std::wstring w_cso_path = fstring_tools::to_utf16(fio::get_shader_file_path(name.c_str()) + ".cso");
      if(FAILED(utils->LoadFile(w_cso_path.c_str(), nullptr, cso_blob.GetAddressOf())))
      {
        LOG_INFO("Unable to find shader in cache or load failed.");
        return false;
      }
      // TODO - This is the only thing missing before I can start using custom lightweight com pointer
      // Error C2440 : '<function-style-cast>': cannot convert from 'const engine::fcom_ptr<IDxcBlobEncoding>' to 'engine::fcom_ptr<IDxcBlob>'
      // It missed the copy constructor:
      // ComPtr(const ComPtr<U> &other, typename Details::EnableIf<Details::IsConvertible<U*, T*>::value, void *>::type * = 0) throw()
      // Possible workaround, use IDxcBlobEncoding instead of IDxcBlob in shader resurce types.
      out_shader_blob = cso_blob;
#if USE_NSIGHT_AFTERMATH
      fapplication::get_instance()->gpu_crash_handler.add_shader_binary(cso_blob.Get());
#endif
    }

    // Load and register pdb file if using aftermath
#if USE_NSIGHT_AFTERMATH
    {
      fcom_ptr<IDxcBlobEncoding> pdb_blob;
      const std::wstring w_pdb_path = fstring_tools::to_utf16(fio::get_shader_file_path(name.c_str()) + ".pdb");
      if(FAILED(utils->LoadFile(w_pdb_path.c_str(), nullptr, pdb_blob.GetAddressOf())))
      {
        LOG_INFO("Unable to find shader debug file in cache or load failed.");
      }
      else
      {
        fapplication::get_instance()->gpu_crash_handler.add_source_shader_debug_data(com_out_shader_blob.Get(), pdb_blob.Get());
      }
    }
#endif
    LOG_INFO("Shader loaded from cache.");
    return true;
  }
  
  // https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll
  // https://simoncoenen.com/blog/programming/graphics/DxcCompiling
  // https://youtu.be/tyyKeTsdtmo?t=1132
  bool fshader_tools::load_and_compile_hlsl(const std::string& hlsl_file_name, const std::string& entrypoint, const std::string& target, fcom_ptr<IDxcBlob>& out_shader_blob, std::string& out_hash)
  {
    fcom_ptr<IDxcUtils> utils;
    fcom_ptr<IDxcCompiler3> compiler;
    fcom_ptr<IDxcIncludeHandler> include_handler;

    const std::string hlsl_file_path = fio::get_shader_file_path(hlsl_file_name.c_str());
    const std::string shader_path = fstring_tools::remove_file_extension(hlsl_file_path) + "_" + entrypoint;
    const std::string shader_name = fstring_tools::remove_file_extension(hlsl_file_name) + "_" + entrypoint;
    out_hash = shader_name;
    
    std::vector<LPCWSTR> arguments;
    const std::wstring w_shader_directory = fstring_tools::to_utf16(fio::get_shaders_dir());
    const std::wstring w_entrypoint = fstring_tools::to_utf16(entrypoint);
    const std::wstring w_target = fstring_tools::to_utf16(target);
    const std::wstring w_hlsl_file_name = fstring_tools::to_utf16(hlsl_file_name);
    arguments.push_back(L"-E");
    arguments.push_back(w_entrypoint.c_str());
    arguments.push_back(L"-T");
    arguments.push_back(w_target.c_str());
    arguments.push_back(L"-I");
    arguments.push_back(w_shader_directory.c_str());
#if BUILD_DEBUG
    arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
    arguments.push_back(DXC_ARG_DEBUG);
    arguments.push_back(L"-Qembed_debug"); // Workaround for Aftermath 2024.2. I save DXC_OUT_OBJECT to .cso file, so it will have symbols.
                                              // Looks like this is related to a bug as confirmed by kleints
                                              // https://github.com/NVIDIA/nsight-aftermath-samples/pull/3#issuecomment-2435892147
    //arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
#endif
    arguments.push_back(w_hlsl_file_name.c_str());
    
    // Initialize the dxc
    // TODO: compiler and utils does not have to be created for each invocation
    //       but keep in mind thread safety: utils, compiler and include handler needs to exist per thread
    if(FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()))))
    {
      LOG_ERROR("Failed to create dxc utils instance.");
      return false;
    }
    if(FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.GetAddressOf()))))
    {
      LOG_ERROR("Failed to create dxc instance.");
      return false;
    }
    if(FAILED(utils->CreateDefaultIncludeHandler(include_handler.GetAddressOf())))
    {
      LOG_ERROR("Failed to create dxc include handler.");
      return false;
    }

    // Load hlsl and compile shader
    fcom_ptr<IDxcResult> dxc_result;
    {
      // Load shader source
      fcom_ptr<IDxcBlobEncoding> source_blob;
      std::wstring w_hlsl_file_path = fstring_tools::to_utf16(hlsl_file_path);
      if(FAILED(utils->LoadFile(w_hlsl_file_path.c_str(), nullptr, source_blob.GetAddressOf())))
      {
        LOG_ERROR("Could not compile shader, failed to load file.");
        return false;
      }

      // Compile shader source
      DxcBuffer source;   
      source.Ptr = source_blob->GetBufferPointer();
      source.Size = source_blob->GetBufferSize();
      source.Encoding = DXC_CP_ACP;
      if(FAILED(compiler->Compile(&source, const_cast<LPCWSTR*>(arguments.data()), fmath::to_uint32(arguments.size()), include_handler.Get(), IID_PPV_ARGS(dxc_result.GetAddressOf()))))
      {
        LOG_ERROR("Could not compile shader, failed to compile");
        return false;
      }
    }

    // Print warnings and errors, fail if errors
    fcom_ptr<IDxcBlobUtf8> errors = nullptr;
    get_dxc_blob(dxc_result.Get(), DXC_OUT_ERRORS, errors);
    if (errors.Get() != nullptr && errors->GetStringLength() != 0)
    {
      LOG_ERROR("Warnings and errors:\n {0}", errors->GetStringPointer());
    }
    HRESULT hr;
    dxc_result->GetStatus(&hr);
    if (FAILED(hr))
    {
      LOG_ERROR("Shader compilation failed: {0}.", hlsl_file_name);
      return false;
    }

    // Object file
    {
      std::string obj_path = shader_path + ".cso";
      if(!get_dxc_blob(dxc_result.Get(), DXC_OUT_OBJECT, out_shader_blob)) { return false; }
      if(!save_dxc_blob(out_shader_blob.Get(), obj_path.c_str())) { return false; }
#if USE_NSIGHT_AFTERMATH
      fapplication::get_instance()->gpu_crash_handler.add_shader_binary(com_out_shader_blob.Get());
#endif
    }

    // Debug symbols
#if BUILD_DEBUG
    {
      fcom_ptr<IDxcBlob> pdb_blob;
      std::string pdb_path = shader_path + ".pdb";
      if(!get_dxc_blob(dxc_result.Get(), DXC_OUT_PDB, pdb_blob)) { return false; }
      if(!save_dxc_blob(pdb_blob.Get(), pdb_path.c_str())) { return false; }
#if USE_NSIGHT_AFTERMATH
      fapplication::get_instance()->gpu_crash_handler.add_source_shader_debug_data(com_out_shader_blob.Get(), pdb_blob.Get());
#endif
    }
#endif
    return true;
  }

  CD3DX12_SHADER_BYTECODE fshader_tools::get_shader_byte_code(IDxcBlob* shader)
  {
    return CD3DX12_SHADER_BYTECODE(shader->GetBufferPointer(), shader->GetBufferSize());
  }

  void fshader_tools::get_blob_pointer_and_size(IDxcBlob* in_blob, uint8_t** out_address, size_t& out_size)
  {
    *out_address = static_cast<uint8_t*>(const_cast<void*>(in_blob->GetBufferPointer()));
    out_size = in_blob->GetBufferSize();
  }

#if USE_FXC
  // https://asawicki.info/news_1719_two_shader_compilers_of_direct3d_12
  // https://github.com/NVIDIAGameWorks/ShaderMake/blob/470bbc7d0c343bc82c988072ee8a1fb2210647ce/src/ShaderMake.cpp#L988
  // https://devblogs.microsoft.com/pix/using-automatic-shader-pdb-resolution-in-pix/
  bool fshader_tools::load_hlsl_fxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, fcom_ptr<ID3D10Blob>& out_shader_blob)
  {
    std::string hlsl_path = fio::get_shader_file_path(file_name.c_str());

    fcom_ptr<ID3DBlob> shader_compiler_errors_blob;
    UINT flags1 = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if BUILD_DEBUG
    flags1 |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG_NAME_FOR_BINARY;
#elif BUILD_RELEASE
    flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
    HRESULT result = D3DCompileFromFile(std::wstring(hlsl_path.begin(), hlsl_path.end()).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
      entrypoint.c_str(), target.c_str(), flags1, 0, out_shader_blob.GetAddressOf(), shader_compiler_errors_blob.GetAddressOf());
    if(FAILED(result))
    {
      if(result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
      {
        LOG_ERROR("Could not compile shader, file {0} not found.", file_name);
      }
      else if(shader_compiler_errors_blob)
      {
        LOG_ERROR("Could not compile shader. {0}", static_cast<const char*>(shader_compiler_errors_blob->GetBufferPointer()));
      }
      else
      {
        LOG_ERROR("Could not compile shader. Result: {0}", result);
      }
      return false;
    }
    
    const char* pdb_file_name = nullptr;
#if BUILD_DEBUG
    // Save the PDB file
    {
      // Retrieve the debug info part of the shader
      fcom_ptr<ID3DBlob> pdb;
      D3DGetBlobPart(out_shader_blob->GetBufferPointer(), out_shader_blob->GetBufferSize(), D3D_BLOB_PDB, 0, &pdb);
      
      // Retrieve the suggested name for the debug data file
      fcom_ptr<ID3DBlob> pdb_name;
      D3DGetBlobPart(out_shader_blob->GetBufferPointer(), out_shader_blob->GetBufferSize(), D3D_BLOB_DEBUG_NAME, 0, &pdb_name);
      
      // This struct represents the first four bytes of the name blob
      struct shader_debug_name
      {
        uint16_t flags;       // Reserved, must be set to zero
        uint16_t name_length;  // Length of the debug name, without null terminator
        // Followed by name_length bytes of the UTF-8-encoded name
        // Followed by a null terminator
        // Followed by [0-3] zero bytes to align to a 4-byte boundary
      };
      
      auto debug_name_data = (const shader_debug_name*)(pdb_name->GetBufferPointer());
      pdb_file_name = (const char*)(debug_name_data + 1);
    
      std::string file = fio::get_shader_file_path(pdb_file_name);
      FILE* fp = fopen(file.c_str(), "wb");
      if (fp)
      {
        fwrite(pdb->GetBufferPointer(), pdb->GetBufferSize(), 1, fp);
        fclose(fp);
      }
    }
#elif BUILD_RELEASE
    // Strip reflection
    fcom_ptr<ID3DBlob> stripped_blob;
    UINT flags = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA;
    D3DStripShader(out_shader_blob->GetBufferPointer(), out_shader_blob->GetBufferSize(), flags, &stripped_blob);
    out_shader_blob = stripped_blob;
#endif

    // Save CSO to the disk
    {
      if(pdb_file_name)
      {
        std::string file = fio::get_shader_file_path(pdb_file_name);
        std::size_t index = file.find_last_of(".");
        file.replace(index, 4, ".cso");
        std::wstring cso_file = std::wstring(file.begin(), file.end());
        D3DWriteBlobToFile(out_shader_blob.Get(), std::wstring(cso_file.begin(), cso_file.end()).c_str(), true);
      }
    }
    return true;
  }
#endif
}