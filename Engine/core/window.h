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

using Microsoft::WRL::ComPtr;

namespace engine
{
  struct fcommand_queue;
  struct fgpu_crash_tracker;
  class hhittable_base;
  
  class ENGINE_API fwindow
  {
  public:   
    void show() const;
    void hide() const;
    virtual void init(WNDPROC wnd_proc, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue);
    virtual void update() = 0;
    virtual void draw(const fcommand_queue* command_queue);
    void present(fgpu_crash_tracker* gpu_crash_handler);
    void resize(const ComPtr<ID3D12Device> device, uint32_t width, uint32_t height);
    virtual const wchar_t* get_name() const = 0;
    virtual void cleanup();

    static constexpr uint32_t back_buffer_count = 2;

    int get_back_buffer_index() const { return back_buffer_index; }
    HWND get_window_handle() const { return hwnd; }
    
    ComPtr<IDXGISwapChain4> swap_chain;
    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
    std::vector<ComPtr<ID3D12Resource>> rtv;
    ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap;
    ComPtr<ID3D12Resource> dsv;
    ComPtr<ID3D12DescriptorHeap> main_descriptor_heap; // srv, cbv, uav
    
  protected:    
    HWND hwnd;
    WNDCLASSEX wc;
    
    bool screen_tearing = false;
    bool vsync = true;
    int width = 1920;
    int height = 1080;
    
    int back_buffer_index = 0;
    hhittable_base* selected_object = nullptr;

  };
}
