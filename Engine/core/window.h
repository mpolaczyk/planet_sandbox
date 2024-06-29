#pragma once

#include <vector>
#include <wrl/client.h>

#include "core/core.h"

struct ID3D12Device;
struct IDXGIFactory4;
struct ID3D12RootSignature;
struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandQueue;

using Microsoft::WRL::ComPtr;

namespace engine
{
  class ENGINE_API fwindow
  {
  public:   
    fwindow(const wchar_t* in_name): name(in_name) {}

    void show() const;
    void hide() const;
    void init(WNDPROC wnd_proc, const ComPtr<ID3D12Device>& in_device, const ComPtr<IDXGIFactory4>& in_factory, const ComPtr<ID3D12CommandQueue>& in_command_queue);
    void present();
    void resize(const ComPtr<ID3D12Device>& in_device, uint32_t in_width, uint32_t in_height);
    void cleanup();

    static constexpr uint32_t back_buffer_count = 3;

  protected:
    const wchar_t* name;
    HWND hwnd;
    WNDCLASSEX wc;
    
    bool screen_tearing = false;
    bool vsync = true;
    uint32_t width = 0;
    uint32_t height = 0;
    
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<IDXGISwapChain3> swap_chain;
    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
    uint32_t rtv_descriptor_size = 0;
    ComPtr<ID3D12DescriptorHeap> srv_descriptor_heap;
    std::vector<ComPtr<ID3D12Resource>> rtv;
    
    uint64_t back_buffer_index = 0;
  };
}
