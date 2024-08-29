#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "editor_window.h"
#include "app/editor_app.h"
#include "renderer/dx12_lib.h"
#include "renderer/command_queue.h"

namespace editor
{
  feditor_app* feditor_window::get_editor_app()
  {
    return static_cast<feditor_app*>(fapplication::instance);  
  }
  
  void feditor_window::init(WNDPROC wnd_proc, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue)
  {
    fwindow::init(wnd_proc,  device, factory,  command_queue);
    
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(hwnd);

    fdx12::create_cbv_srv_uav_descriptor_heap(device, ui_descriptor_heap);
#if BUILD_DEBUG
    ui_descriptor_heap->SetName(L"UI descriptor Heap");
#endif
    
    ImGui_ImplDX12_Init( device.Get(), back_buffer_count, DXGI_FORMAT_R8G8B8A8_UNORM,
      ui_descriptor_heap.Get(),
      ui_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
      ui_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
    
    // TO FIX - it crashes in second frame because imgui knows only the first descriptor heap, second frame uses the second one...
    // Or use one descriptor heap... wierd
    get_editor_app()->load_window_state();
  }

  void feditor_window::cleanup()
  {
    get_editor_app()->save_window_state();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
  }

  void feditor_window::update()
  {
    handle_input();
    update_default_spawn_position();
  }

  void feditor_window::draw()
  {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
    if (0) { ImGui::ShowDemoWindow(); }
#endif

    draw_editor_window(editor_window_model);
    draw_scene_window(scene_window_model);
  }

  void feditor_window::render(const fcommand_queue* command_queue)
  {
    fwindow::render(command_queue);
    
    ComPtr<ID3D12GraphicsCommandList> command_list = command_queue->get_command_list(ecommand_list_type::ui, back_buffer_index);
    ComPtr<ID3D12Device2> device = fapplication::instance->device;

    fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    fdx12::set_render_targets(device, command_list, dsv_descriptor_heap, rtv_descriptor_heap, back_buffer_index);
    fdx12::set_viewport(command_list, width, height);
    fdx12::set_scissor(command_list, width, height);
    
    command_list->SetDescriptorHeaps(1, ui_descriptor_heap.GetAddressOf());

    ImGui::Render();
#if RENDER_IMGUI
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());
#endif
    
    fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  }


}