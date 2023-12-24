
#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

#include "engine/log.h"

namespace engine
{
  OBJECT_DEFINE(gpu_renderer, async_renderer_base, GPU renderer)
  OBJECT_DEFINE_SPAWN(gpu_renderer)

  void gpu_renderer::job_init()
  {
    // FIX temporary hack here! It should be properly persistent as part fo the scene (not hittable)
    vertex_shader.set_name("gpu_renderer_vs");
    vertex_shader.get();
    pixel_shader.set_name("gpu_renderer_ps");
    pixel_shader.get();

    
    ID3D11InputLayout* input_layout;
    {
      D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
      {
        { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
      };
      const auto blob = vertex_shader.get()->shader_blob;
      HRESULT result = dx11::instance().device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc),
        blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout);
      if (FAILED(result))
      {
        LOG_ERROR("Unable to create input layout.");
      }
    }
  }
  
  void gpu_renderer::job_update()
  {
    
  }

  void gpu_renderer::job_cleanup()
  {
    
  }
}
