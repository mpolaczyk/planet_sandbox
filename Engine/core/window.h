#pragma once

#include <dxgi1_6.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "core/core.h"
#include "renderer/descriptor_heap.h"

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
    virtual void init(WNDPROC wnd_proc, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue);
    virtual void update() = 0;
    virtual void draw(std::shared_ptr<fcommand_queue> command_queue);
    void present(fgpu_crash_tracker* gpu_crash_handler);
    void resize(uint32_t width, uint32_t height);
    virtual const wchar_t* get_name() const = 0;
    virtual void cleanup();

    static constexpr uint32_t back_buffer_count = 2;

    uint32_t get_back_buffer_index() const { return back_buffer_index; }
    HWND get_window_handle() const { return hwnd; }
    
    ComPtr<IDXGISwapChain4> swap_chain;
    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
    fdescriptor_heap dsv_descriptor_heap;
    std::vector<ComPtr<ID3D12Resource>> rtv;            // index is backbuffer id
    ComPtr<ID3D12Resource> dsv;
    fdescriptor_heap main_descriptor_heap; // srv, cbv, uav
    
  protected:    
    HWND hwnd;
    WNDCLASSEX wc;
    
    bool screen_tearing = false;
    bool vsync = true;
    uint32_t width = 1920;
    uint32_t height = 1080;
    
    uint32_t back_buffer_index = 0;
    hhittable_base* selected_object = nullptr;

  };
}
