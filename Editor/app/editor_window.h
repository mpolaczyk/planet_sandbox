#pragma once

#include "ui/ui.h"
#include "core/window.h"

#include "reactphysics3d/collision/RaycastInfo.h"

namespace reactphysics3d
{
  class RigidBody;  
}

namespace engine
{
  class hscene;
  struct fcommand_queue;
}

namespace editor
{
  class feditor_app;

  class raycast_callback : public reactphysics3d::RaycastCallback
  {
  public:
    reactphysics3d::Body* get_closest_body() const
    {
      return closest_body;
    };

    reactphysics3d::Vector3 get_world_point() const
    {
      return world_point;
    }
    
  protected:
    virtual float notifyRaycastHit(const reactphysics3d::RaycastInfo& info) override;

  private:
    float smallest_fraction = 1.0f;
    reactphysics3d::Body* closest_body = nullptr;
    reactphysics3d::Vector3 world_point;
  };
  
  class feditor_window final : public engine::fwindow
  {
  public:
    CTOR_DEFAULT(feditor_window)
    CTOR_MOVE_COPY_DELETE(feditor_window)
    virtual ~feditor_window() override;

    virtual void init(WNDPROC wnd_proc, ComPtr<IDXGIFactory4> factory, const wchar_t* name) override;
    virtual void update() override;
    virtual void draw() override;

    void handle_input();
    void update_default_spawn_position();
    
    // Runtime state
    feditor_window_model editor_window_model;
    fscene_window_model scene_window_model;

    reactphysics3d::RigidBody* selected_body;
    
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

    engine::fdescriptor_heap ui_descriptor_heap; // srv, cbv, uav
  };
}