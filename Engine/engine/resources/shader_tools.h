#pragma once

#include <string>

#include "core/core.h"
#include "core/com_pointer.h"
#include "engine/renderer/graphics_pipeline.h"

struct IDxcResult;
struct IDxcBlobUtf8;
struct IDxcBlob;
struct CD3DX12_SHADER_BYTECODE;
enum DXC_OUT_KIND;

namespace engine
{
  struct ENGINE_API fshader_tools
  {
    static bool load_compiled_shader(const std::string& file_name, fcom_ptr<IDxcBlob>& out_shader_blob);
  
    static bool load_and_compile_hlsl(const std::string& hlsl_file_name, const std::string& entrypoint, const std::string& target, fcom_ptr<IDxcBlob>& out_shader_blob, std::string& out_has);

    static bool get_dxc_hash(IDxcResult* result, std::string& out_hash);
    static bool get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, fcom_ptr<IDxcBlob>& out_blob);
    static bool get_dxc_blob(IDxcResult* result, DXC_OUT_KIND blob_type, fcom_ptr<IDxcBlobUtf8>& out_blob);
    static bool save_dxc_blob(IDxcBlob* blob, const char* path);

    static CD3DX12_SHADER_BYTECODE get_shader_byte_code(IDxcBlob* shader);
    static void get_blob_pointer_and_size(IDxcBlob* in_blob, uint8_t** out_address, size_t& out_size);
    static void copy(fcom_ptr<IDxcBlob>& destination_blob, const fcom_ptr<IDxcBlob>& source_blob);


#if USE_FXC
    static bool ENGINE_API load_hlsl_fxc(const std::string& file_name, const std::string& entrypoint, const std::string& target, fcom_ptr<ID3D10Blob>& out_shader_blob);
#endif
  };
}
