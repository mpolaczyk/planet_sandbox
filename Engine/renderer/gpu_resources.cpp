#include "gpu_resources.h"

#include "renderer/dx12_lib.h"

namespace engine
{
  void fbuffer::upload(const void* data) const
  {
    fdx12::upload_host_buffer(resource.Get(), size, data);
  }

  void ftexture_resource::release()
  {
    srv.release();
    rtv.release();
    dsv.release();
    DX_RELEASE(com)
    DX_RELEASE(upload_com)
  }
}