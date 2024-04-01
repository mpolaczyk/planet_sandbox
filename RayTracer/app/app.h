#pragma once

#include <functional>

#include "math/camera.h"
#include "renderer/renderer_base.h"
#include "ui/ui.h"

namespace engine
{
  class renderer_base;
  class material_asset;
  class hittable;
  class scene;
}

namespace ray_tracer
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
    fcamera_config camera_conf;

    // Rendering state
    frenderer_config renderer_conf;
  
    // OS window state
    fwindow_config window_conf;

    // Imgui window states
    fraytracer_window_model rw_model;
    foutput_window_model ow_model;
    fscene_editor_window_model sew_model;

    // Runtime state
    bool is_running = true;
    int output_width = 0;
    int output_height = 0;
    rrenderer_base* renderer = nullptr;
  
    fvec3 center_of_scene;
    float distance_to_center_of_scene = 0.0f;

    float output_window_lmb_x = -1.0f;
    float output_window_lmb_y = -1.0f;
    hhittable_base* selected_object = nullptr;

    void load_scene_state();
    void save_scene_state();
    void load_rendering_state();
    void save_rendering_state();
    void load_assets();
    void save_materials();
    void load_window_state();
    void save_window_state();
  };

  // FIX Move this to app_instance class
  void draw_camera_panel(fcamera_panel_model& model, fapp_instance& state);
  void draw_renderer_panel(frenderer_panel_model& model, fapp_instance& state);
  void draw_managed_objects_panel(fapp_instance& state);
  void draw_hotkeys_panel(fapp_instance& state);
  void draw_raytracer_window(fraytracer_window_model& model, fapp_instance& state);
  void draw_output_window(foutput_window_model& model, fapp_instance& state);
  void draw_scene_editor_window(fscene_editor_window_model& model, fapp_instance& state);
  void draw_new_object_panel(fnew_object_panel_model& model, fapp_instance& state);

  template<typename T>
  void draw_selection_combo(fselection_combo_model<T>& model, fapp_instance& state, const char* name, std::function<bool(const T*)> predicate, const T* default_selected_object = nullptr);

  void draw_delete_object_panel(fdelete_object_panel_model& model, fapp_instance& state);

  void update_default_spawn_position(fapp_instance& state);

  void handle_input(fapp_instance& state);
}