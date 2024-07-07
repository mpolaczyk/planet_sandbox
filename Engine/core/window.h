#pragma once

#include <dxgi1_6.h>
#include <vector>
#include <wrl/client.h>

#include "core/core.h"

struct ID3D12Device;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;

using Microsoft::WRL::ComPtr;

namespace engine
{
  class ENGINE_API fwindow
  {
  public:   
    void show() const;
    void hide() const;
    virtual void init(WNDPROC wnd_proc, const ComPtr<ID3D12Device>& in_device, const ComPtr<IDXGIFactory4>& in_factory, const ComPtr<ID3D12CommandQueue>& in_command_queue);
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void render(const ComPtr<ID3D12GraphicsCommandList>& command_list);
    void present();
    void resize(const ComPtr<ID3D12Device>& in_device, uint32_t in_width, uint32_t in_height);
    virtual const wchar_t* get_name() const = 0;
    virtual void cleanup();

    static constexpr uint32_t back_buffer_count = 3;

    int get_back_buffer_index() const { return back_buffer_index; }

  protected:
    HWND hwnd;
    WNDCLASSEX wc;
    
    bool screen_tearing = false;
    bool vsync = true;
    int width = 1920;
    int height = 1080;
    
    //ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<IDXGISwapChain4> swap_chain;
    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
    uint32_t rtv_descriptor_size = 0;
    ComPtr<ID3D12DescriptorHeap> srv_descriptor_heap;
    std::vector<ComPtr<ID3D12Resource>> rtv;
    
    int back_buffer_index = 0;
  };
}
