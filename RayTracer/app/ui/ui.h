#pragma once

namespace engine
{
  class renderer_base;
}

struct window_config
{
public:
  int x = 100;
  int y = 100;
  int w = 1920;
  int h = 1080;
};

/*
 *_model - for each panel/window
 - not part of app state
 - owned by UI
 - required to maintain panel/window state between frames
 - not needed to be shared between multiple panels/widgets
 - not persistent
*/
struct camera_panel_model
{
  
};

template<typename T>
struct selection_combo_model
{
  // In
  std::vector<const T*> objects;
  // Out, selected by the widget
  int selected_id = 0;
  const T* selected_object = nullptr;

  void reset()
  {
    selected_object = nullptr;
    selected_id = -1;
  }
};

struct hittable_selection_combo_model
{
  int selected_id = 0;
};

struct material_selection_combo_model
{
  int selected_id = 0;
};

struct renderer_selection_combo_model
{
  int selected_id = 0;
};

struct renderer_panel_model
{
  bool render_pressed = false;
  selection_combo_model<material_asset> m_model;
  selection_combo_model<class_object> r_model;
};

struct raytracer_window_model
{
  renderer_panel_model rp_model;
};

struct output_window_model
{
  float zoom = 1.0f;
  bool auto_render = false;
};

struct new_object_panel_model
{
  selection_combo_model<class_object> c_model;
};

struct delete_object_panel_model
{
  int selected_id = 0;
};

struct scene_editor_window_model
{
  int selected_id = -1;
  camera_panel_model cp_model;
  new_object_panel_model nop_model;
  delete_object_panel_model d_model;
  selection_combo_model<material_asset> m_model;
};
