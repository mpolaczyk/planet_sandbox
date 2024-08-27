#pragma once

#include "app/ui/ui.h"
#include "core/window.h"

namespace engine
{
  class hscene;
}

namespace editor
{
  class feditor_app;
  
  class feditor_window final : public engine::fwindow
  {
  public:
    virtual void init(WNDPROC wnd_proc, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> command_queue) override;
    virtual const wchar_t* get_name() const override { return L"Editor"; }
    virtual void cleanup() override;
    virtual void update() override;
    virtual void draw() override;
    virtual void render(ComPtr<ID3D12GraphicsCommandList> command_list) override;

    void handle_input();
    void update_default_spawn_position();
    
    // Runtime state
    feditor_window_model editor_window_model;
    foutput_window_model output_window_model;
    fscene_window_model scene_window_model;
    hhittable_base* selected_object = nullptr;

  private:
    static feditor_app* get_editor_app();
    
    // Editor window
    void draw_editor_window(feditor_window_model& model);
    void draw_hotkeys_panel();
    void draw_materials_panel(fmaterials_panel_model& model);
    void draw_object_registry_panel();
    
    // Scene window
    void draw_scene_window(fscene_window_model& model);
    void draw_renderer_panel(frenderer_panel_model& model);
    void draw_camera_panel();
    void draw_scene_panel();
    void draw_scene_objects_panel(fobjects_panel_model& model);

    // Output window
    void draw_output_window(foutput_window_model& model);
    void draw_new_object_panel(fnew_object_panel_model& model);
    void draw_delete_object_panel(fdelete_object_panel_model& model);
    
    // Runtime state
    fvec3 center_of_scene;
    float distance_to_center_of_scene = 0.0f;
    bool output_window_is_clicked = false;
    bool output_window_is_hovered = false;
    uint8_t output_window_cursor_color[3] = {0};
    
  };
}