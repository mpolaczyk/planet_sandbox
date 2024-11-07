#include "gpu_resources.h"

#include <format>
#include <string>

#include "renderer/dx12_lib.h"

namespace engine
{
  void fbuffer::upload(const void* data) const
  {
    fdx12::upload_buffer(resource, size, data);
  }

  void ftexture_resource::upload(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, const void* data)
  {
    // TODO nonsense! so much copying... you need to clean this
    fdx12::upload_texture(device, command_list, width, height, channels, format, data, element_size, resource, resource_upload);
    is_online = true;
  }
}