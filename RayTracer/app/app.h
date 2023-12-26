#pragma once

#include <functional>

#include "math/camera.h"
#include "renderer/async_renderer_base.h"
#include "ui/ui.h"

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace engine
{
  class async_renderer_base;
  class material_asset;
  class hittable;
  class scene;
}

/*
   app_state - root structure for the application
   - accessible from multiple panels/widgets
   - holds resources
   - persistent
*/
class app_instance
{
public:
  app_instance();
  ~app_instance();

  // Scene state
  scene* scene_root = nullptr;
  camera_config camera_conf;

  // Rendering state
  renderer_config renderer_conf;
  
  // OS window state
  window_config window_conf;

  // Imgui window states
  raytracer_window_model rw_model;
  output_window_model ow_model;
  scene_editor_window_model sew_model;

  // Runtime state
  bool is_running = true;
  int output_width = 0;
  int output_height = 0;
  async_renderer_base* renderer = nullptr;

  // FIX used only for cpu renderer...
  ID3D11ShaderResourceView* output_srv = nullptr;
  ID3D11Texture2D* output_texture = nullptr;
  
  vec3 center_of_scene;
  float distance_to_center_of_scene = 0.0f;

  float output_window_lmb_x = -1.0f;
  float output_window_lmb_y = -1.0f;
  hittable* selected_object = nullptr;
  float move_speed = 5.0f;

  void load_scene_state();
  void save_scene_state();
  void load_rendering_state();
  void save_rendering_state();
  void load_assets();
  void save_materials();
  void load_window_state();
  void save_window_state();
};

// TODO: Move this to app_instance class
void draw_camera_panel(camera_panel_model& model, app_instance& state);
void draw_renderer_panel(renderer_panel_model& model, app_instance& state);
void draw_managed_objects_panel(app_instance& state);
void draw_hotkeys_panel(app_instance& state);
void draw_raytracer_window(raytracer_window_model& model, app_instance& state);
void draw_output_window(output_window_model& model, app_instance& state);
void draw_scene_editor_window(scene_editor_window_model& model, app_instance& state);
void draw_new_object_panel(new_object_panel_model& model, app_instance& state);

template<typename T>
void draw_selection_combo(selection_combo_model<T>& model, app_instance& state, const char* name, std::function<bool(const T*)> predicate, const T* default_selected_object = nullptr);

void draw_delete_object_panel(delete_object_panel_model& model, app_instance& state);

void update_default_spawn_position(app_instance& state);

void handle_input(app_instance& state);