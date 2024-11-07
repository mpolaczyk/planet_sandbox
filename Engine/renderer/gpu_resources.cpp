#include "gpu_resources.h"

#include "renderer/dx12_lib.h"

namespace engine
{
  void fbuffer::upload(const void* data) const
  {
    fdx12::upload_buffer(resource, size, data);
  }
}