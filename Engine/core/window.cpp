
#include <dxgi1_6.h>
#include "d3d12.h"
#include "d3dx12/d3dx12_root_signature.h"

#include "core/window.h"
#include "core/exceptions.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  void fwindow::show() const
  {
    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);
  }

  void fwindow::hide() const 
  {
    ::ShowWindow(hwnd, SW_HIDE);
  }

  void fwindow::init(WNDPROC wnd_proc, const ComPtr<ID3D12Device>& in_device, const ComPtr<IDXGIFactory4>& in_factory, const ComPtr<ID3D12CommandQueue>& in_command_queue)
  {
    wc = {sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, name, NULL};
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, name, WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    
    screen_tearing = fdx12::enable_screen_tearing(in_factory);
    fdx12::create_swap_chain(hwnd, in_factory, in_command_queue, back_buffer_count, screen_tearing, swap_chain);
    fdx12::create_render_target(in_device, swap_chain, back_buffer_count, rtv_descriptor_heap, rtv_descriptor_size, rtv);
    fdx12::create_shader_resource(in_device, srv_descriptor_heap);
    fdx12::create_root_signature(in_device, root_signature);
  }

  void fwindow::present()
  {
    uint32_t present_sync = vsync ? 1 : 0;
    uint32_t present_flags = screen_tearing && !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    THROW_IF_FAILED(swap_chain->Present(present_sync, present_flags))

    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
  }
  
  void fwindow::resize(const ComPtr<ID3D12Device>& in_device, uint32_t in_width, uint32_t in_height)
  {
    if(in_width == 0 || in_height == 0) { return; }
    if(in_width == width && in_height == height) { return; }
    
    // Release resources
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      rtv[n].Reset();
    }

    // Resize
    THROW_IF_FAILED(swap_chain->ResizeBuffers(0, in_width, in_height, DXGI_FORMAT_UNKNOWN, 0))

    back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

    // Create resources
    {
      CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        THROW_IF_FAILED(swap_chain->GetBuffer(n, IID_PPV_ARGS(&rtv[n])))
        in_device->CreateRenderTargetView(rtv[n].Get(), nullptr, handle);
        handle.Offset(rtv_descriptor_size);
      }
    }
    width = in_width;
    height = in_height;
  }
  
  void fwindow::cleanup()
  {
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      rtv[n].Reset();
    }
    DX_RELEASE(swap_chain);
    DX_RELEASE(rtv_descriptor_heap);
    DX_RELEASE(srv_descriptor_heap);
    DX_RELEASE(root_signature);

    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
}