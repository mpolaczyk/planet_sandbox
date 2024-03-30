#pragma once

namespace engine
{
  class renderer_base;
}

namespace ray_tracer
{
  struct fwindow_config
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
  struct fcamera_panel_model
  {
  
  };

  template<typename T>
  struct fselection_combo_model
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

  struct fhittable_selection_combo_model
  {
    int selected_id = 0;
  };

  struct fmaterial_selection_combo_model
  {
    int selected_id = 0;
  };

  struct frenderer_selection_combo_model
  {
    int selected_id = 0;
  };

  struct frenderer_panel_model
  {
    bool render_pressed = false;
    fselection_combo_model<amaterial> m_model;
    fselection_combo_model<oclass_object> r_model;
  };

  struct fraytracer_window_model
  {
    frenderer_panel_model rp_model;
  };

  struct foutput_window_model
  {
    float zoom = 1.0f;
    bool auto_render = false;
  };

  struct fnew_object_panel_model
  {
    fselection_combo_model<oclass_object> c_model;
  };

  struct fdelete_object_panel_model
  {
    int selected_id = 0;
  };

  struct fscene_editor_window_model
  {
    int selected_id = -1;
    fcamera_panel_model cp_model;
    fnew_object_panel_model nop_model;
    fdelete_object_panel_model d_model;
    fselection_combo_model<amaterial> m_model;
  };
}