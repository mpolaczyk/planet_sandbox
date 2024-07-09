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
    virtual void init(WNDPROC wnd_proc, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue);
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void render(ComPtr<ID3D12GraphicsCommandList> command_list);
    void present();
    void resize(const ComPtr<ID3D12Device> device, uint32_t width, uint32_t height);
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
    
    ComPtr<IDXGISwapChain4> swap_chain;
    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
    ComPtr<ID3D12DescriptorHeap> srv_descriptor_heap;
    ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap;
    std::vector<ComPtr<ID3D12Resource>> rtv;
    ComPtr<ID3D12Resource> dsv;
    
    int back_buffer_index = 0;
  };
}
