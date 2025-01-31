#pragma once

#include "app/ui/ui.h"

namespace engine
{
  class hscene;
  struct fcommand_queue;
}

namespace editor
{
  class feditor_app;
  
  class feditor_window final : public engine::fwindow
  {
  public:
    CTOR_DEFAULT(feditor_window)
    CTOR_MOVE_COPY_DELETE(feditor_window)
    virtual ~feditor_window() override;

    virtual void init(WNDPROC wnd_proc, IDXGIFactory4* factory, const wchar_t* name) override;
    virtual void update() override;
    virtual void draw() override;

    void handle_input();
    void update_default_spawn_position();
    
    // Runtime state
    feditor_window_model editor_window_model;
    fscene_window_model scene_window_model;
    
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
    void draw_new_object_panel(fnew_object_panel_model& model);
    void draw_delete_object_panel(fdelete_object_panel_model& model);
    
    // Runtime state
    engine::fvec3 object_spawn_location;

    // Camera smooting
    int32_t yaw = 0;
    int32_t pitch = 0;
    int32_t last_yaw = 0;
    int32_t last_pitch = 0;
    
    engine::fdescriptor_heap ui_descriptor_heap; // srv, cbv, uav
  };
}