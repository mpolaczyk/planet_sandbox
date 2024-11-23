#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "editor_window.h"
#include "app/editor_app.h"
#include "renderer/dx12_lib.h"
#include "renderer/command_queue.h"
#include "renderer/device.h"

namespace editor
{
  feditor_app* feditor_window::get_editor_app()
  {
    return static_cast<feditor_app*>(fapplication::get_instance());  
  }

  feditor_window::~feditor_window()
  {
    get_editor_app()->save_window_state();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
  }

  void feditor_window::init(WNDPROC wnd_proc, ComPtr<IDXGIFactory4> factory, const wchar_t* name)
  {
    fwindow::init(wnd_proc, factory, name);
    
    fdevice* device = fapplication::get_instance()->device.get();

    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(hwnd);

    device->create_cbv_srv_uav_descriptor_heap(ui_descriptor_heap, "UI descriptor_heap");
    
    ImGui_ImplDX12_Init(device->com.Get(), back_buffer_count, DXGI_FORMAT_R8G8B8A8_UNORM,
      ui_descriptor_heap.com.Get(),
      ui_descriptor_heap.com->GetCPUDescriptorHandleForHeapStart(),
      ui_descriptor_heap.com->GetGPUDescriptorHandleForHeapStart());
    
    // TO FIX - it crashes in second frame because imgui knows only the first descriptor heap, second frame uses the second one...
    // Or use one descriptor heap... wierd
    get_editor_app()->load_window_state();
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

    fwindow::draw();

    std::shared_ptr<fcommand_queue> command_queue = fapplication::get_instance()->command_queue;
    std::shared_ptr<fgraphics_command_list> command_list = command_queue->get_command_list(ecommand_list_purpose::ui, back_buffer_index);

    command_list->resource_barrier(rtv[back_buffer_index].com.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    command_list->set_render_targets(rtv[back_buffer_index], &dsv);
    command_list->set_viewport(width, height);
    command_list->set_scissor(width, height);
    
    command_list->com->SetDescriptorHeaps(1, ui_descriptor_heap.com.GetAddressOf());

    ImGui::Render();
#if RENDER_IMGUI
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list->com.Get());
#endif
    
    command_list->resource_barrier(rtv[back_buffer_index].com.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  }


}