#include "gpu_resources.h"

#include <format>
#include <string>

#include "renderer/dx12_lib.h"

namespace engine
{
  // TODO create a device class and move all create() there
  fconst_buffer fconst_buffer::create(ID3D12Device* device, fdescriptor_heap* heap, uint64_t in_size, const char* name)
  {
    fconst_buffer temp;
    temp.size = in_size;
    temp.cbv = *heap->push();
    fdx12::create_const_buffer(device, temp.cbv.cpu_handle, temp.size, temp.resource);
#if BUILD_DEBUG
    DX_SET_NAME(temp.resource, "{}", name)
#endif
    return temp;
  }

  fshader_resource_buffer fshader_resource_buffer::create(ID3D12Device* device, fdescriptor_heap* heap, uint64_t in_size, const char* name)
  {
    fshader_resource_buffer temp;
    temp.size = in_size;
    temp.srv = *heap->push();
    fdx12::create_shader_resource_buffer(device, temp.srv.cpu_handle, temp.size, temp.resource);
#if BUILD_DEBUG
    DX_SET_NAME(temp.resource, "{}", name)
#endif
    return temp;
  }

  void fbuffer::upload(const void* data) const
  {
    fdx12::upload_buffer(resource, size, data);
  }
  
  ftexture_resource ftexture_resource::create(ID3D12Device* device, fdescriptor_heap* heap, uint32_t width, uint32_t height, uint32_t channels, uint32_t element_size, DXGI_FORMAT format, const char* name)
  {
    ftexture_resource temp;
    temp.height = height;
    temp.width = width;
    temp.format = format;
    temp.channels = channels;
    temp.element_size = element_size;
    temp.srv = *heap->push();
    fdx12::create_texture_resource(device, width, height, format, temp.srv.cpu_handle, temp.resource, temp.resource_upload);
#if BUILD_DEBUG
    DX_SET_NAME(temp.resource, "Texture: {}", name)
    DX_SET_NAME(temp.resource_upload, "Texture upload: {}", name)
#endif
    return temp;
  }

  void ftexture_resource::upload(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, const void* data)
  {
    // TODO nonsense! so much copying... you need to clean this
    fdx12::upload_texture(device, command_list, width, height, channels, format, data, element_size, resource, resource_upload);
    is_online = true;
  }
}