#pragma once

#include "math/camera.h"
#include "renderer/renderer_base.h"
#include "ui/ui.h"

namespace engine
{
  class renderer_base;
  class material_asset;
  class hhittable_base;
  class scene;
}

namespace editor
{
  /*
     app_state - root structure for the application
     - accessible from multiple panels/widgets
     - holds resources
     - persistent
  */
  class fapp_instance final
  {
  public:
    fapp_instance();
    ~fapp_instance();

    // Scene state
    hscene* scene_root = nullptr;
    fcamera camera;

    // OS window state
    fwindow_config window_conf;

    // Imgui window states
    feditor_window_model editor_window_model;
    foutput_window_model output_window_model;
    fscene_window_model scene_window_model;

    // Runtime state
    bool is_running = true;
    float app_delta_time_ms = 0.0f;
    float render_delta_time_ms = 0.0f;
    fvec3 center_of_scene;
    float distance_to_center_of_scene = 0.0f;
    
    bool output_window_is_clicked = false;
    bool output_window_is_hovered = false;
    uint8_t output_window_cursor_color[3];
    hhittable_base* selected_object = nullptr;

    void load_scene_state();
    void save_scene_state();
    void load_assets();
    void save_materials();
    void load_window_state();
    void save_window_state();
  };

  // FIX Move this to app_instance class

  // Editor window
  void draw_editor_window(feditor_window_model& model, fapp_instance& state);
  void draw_hotkeys_panel(fapp_instance& state);
  void draw_materials_panel(fmaterials_panel_model& model, fapp_instance& state);
  void draw_object_registry_panel();

  // Scene window
  void draw_scene_window(fscene_window_model& model, fapp_instance& state);
  void draw_renderer_panel(frenderer_panel_model& model, fapp_instance& state);
  void draw_camera_panel(fapp_instance& state);
  void draw_scene_panel(fapp_instance& state);
  void draw_scene_objects_panel(fobjects_panel_model& model, fapp_instance& state);

  // Output window
  void draw_output_window(foutput_window_model& model, fapp_instance& state);

  
  void draw_new_object_panel(fnew_object_panel_model& model, fapp_instance& state);

  void draw_delete_object_panel(fdelete_object_panel_model& model, fapp_instance& state);

  void update_default_spawn_position(fapp_instance& state);

  void handle_input(fapp_instance& state);
}