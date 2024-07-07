#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "editor_window.h"
#include "app/editor_app.h"
#include "renderer/dx12_lib.h"

namespace editor
{
  feditor_app* feditor_window::get_editor_app()
  {
    return static_cast<feditor_app*>(fapplication::instance);  
  }
  
  void feditor_window::init(WNDPROC wnd_proc, const ComPtr<ID3D12Device>& in_device, const ComPtr<IDXGIFactory4>& in_factory, const ComPtr<ID3D12CommandQueue>& in_command_queue)
  {
    fwindow::init(wnd_proc, in_device, in_factory, in_command_queue);
    
    ImGui::StyleColorsClassic();
    
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(in_device.Get(), back_buffer_count, DXGI_FORMAT_R8G8B8A8_UNORM, srv_descriptor_heap.Get(), srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
    
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
    //draw_output_window(output_window_model);
    draw_scene_window(scene_window_model);

    ImGui::Render();
  }

  void feditor_window::render(const ComPtr<ID3D12GraphicsCommandList>& command_list)
  {
    fwindow::render(command_list);

    fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());

    fdx12::resource_barrier(command_list, rtv[back_buffer_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  }


}