#pragma once

#include <dxgi1_6.h>
#include <vector>

#include "core/core.h"
#include "core/com_pointer.h"
#include "engine/renderer/descriptor_heap.h"
#include "engine/renderer/gpu_resources.h"

struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandQueue;
struct IDXGIFactory4;

namespace engine
{
  struct fcommand_queue;
  struct fgpu_crash_tracker;
  class hhittable_base;
  
  class ENGINE_API fwindow
  {
  public:
    CTOR_DEFAULT(fwindow)
    CTOR_MOVE_COPY_DELETE(fwindow)
    virtual ~fwindow();

    virtual void init(WNDPROC wnd_proc, IDXGIFactory4* factory, const wchar_t* name);
    void show() const;
    void hide() const;
    virtual void update() = 0;
    virtual void draw();
    void present(fgpu_crash_tracker* gpu_crash_handler);
    void request_resize(uint32_t in_width, uint32_t in_height);
    bool try_apply_resize();
    uint32_t get_width() const;
    uint32_t get_height() const;
    
    static constexpr uint32_t back_buffer_count = 2;

    uint32_t get_back_buffer_index() const { return back_buffer_index; }
    HWND get_window_handle() const { return hwnd; }
    
    fcom_ptr<IDXGISwapChain4> swap_chain;

    // TODO move heaps to the fdevice
    fdescriptor_heap rtv_descriptor_heap;
    fdescriptor_heap dsv_descriptor_heap;
    fdescriptor_heap main_descriptor_heap; // srv, cbv, uav

    std::vector<ftexture_resource> rtv;        // index is back buffer id
    ftexture_resource dsv;

  protected:    
    HWND hwnd{};
    WNDCLASSEX wc{};
    
    bool screen_tearing = false;
    bool vsync = true;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t requested_width = 1920;
    uint32_t requested_height = 1080;
    
    uint32_t back_buffer_index = 0;
    hhittable_base* selected_object = nullptr;

  };
}
