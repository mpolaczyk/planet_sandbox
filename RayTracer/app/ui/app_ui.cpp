#include "stdafx.h"

#include "draw_edit_panel.h"
#include "imgui.h"

#include "app/app.h"
#include "math/chunk_generator.h"
#include "renderer/renderer_base.h"
#include "math/camera.h"

#include "core/core.h"
#include "engine.h"
#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/scene.h"
#include "renderers/cpu_renderer.h"
#include "renderers/gpu_renderer.h"

namespace ray_tracer
{
  void draw_raytracer_window(fraytracer_window_model& model, fapp_instance& state)
  {
    ImGui::Begin("RAYTRACER", nullptr);

    if (ImGui::MenuItem("SAVE STATE"))
    {
      state.save_rendering_state();
      state.save_materials();
    }
    ImGui::Separator();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    draw_hotkeys_panel(state);
    draw_renderer_panel(model.rp_model, state);
    draw_managed_objects_panel(state);
    ImGui::End();
  }

  void draw_camera_panel(fcamera_panel_model& model, fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAMERA");
    ImGui::Separator();
    float ar[2] = { state.camera_conf.aspect_ratio_w, state.camera_conf.aspect_ratio_h };
    ImGui::InputFloat2("Aspect ratio", ar);
    state.camera_conf.aspect_ratio_w = ar[0];
    state.camera_conf.aspect_ratio_h = ar[1];
    ImGui::Text("Aspect ratio = %.3f", state.camera_conf.aspect_ratio_w / state.camera_conf.aspect_ratio_h);
    ImGui::InputFloat("Field of view", &state.camera_conf.field_of_view, 1.0f, 189.0f, "%.0f");
    ImGui::InputFloat("Projection", &state.camera_conf.type, 0.1f, 1.0f, "%.2f");
    ImGui::Text("0 = Perspective; 1 = Orthografic");
    ImGui::Separator();
    ImGui::InputFloat3("Look from", state.camera_conf.look_from.e, "%.2f");
    ImGui::InputFloat("Pitch", &state.camera_conf.pitch, 0.1f, 1.0f, "%.2f");
    ImGui::InputFloat("Yaw", &state.camera_conf.yaw, 0.1f, 1.0f, "%.2f");
    ImGui::Separator();
    ImGui::InputFloat("Focus distance", &state.camera_conf.dist_to_focus, 0.0f, 1000.0f, "%.2f");
    ImGui::InputFloat("Aperture", &state.camera_conf.aperture, 0.1f, 1.0f, "%.2f");
  }

  void draw_renderer_panel(frenderer_panel_model& model, fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RENDERER");
  
    ImGui::Separator();
  
    // FIX this does not work for class objects!
    // co->is_child_of(renderer_base::get_class_static());
    model.r_model.objects = REG.find_all<const oclass_object>([](const oclass_object* co) -> bool { return co->get_parent_class_name() == rcpu::get_class_static()->get_class_name(); });  
    model.r_model.objects.push_back(rgpu::get_class_static());
  
    draw_selection_combo<oclass_object>(model.r_model, state, "Renderer class",
      [=](const oclass_object* obj) -> bool { return true; }, state.renderer_conf.type);
    if(model.r_model.selected_object != state.renderer_conf.type)
    {
      state.renderer_conf.new_type = model.r_model.selected_object;
    }
    ImGui::Separator();
  
    ImGui::InputInt("Resolution v", &state.renderer_conf.resolution_vertical, 1, 2160);
    state.renderer_conf.resolution_horizontal = (int)((float)state.renderer_conf.resolution_vertical * state.camera_conf.aspect_ratio_w / state.camera_conf.aspect_ratio_h);
    ImGui::Text("Resolution h = %d", state.renderer_conf.resolution_horizontal);

    ImGui::Separator();

    ImGui::InputInt("Rays per pixel", &state.renderer_conf.rays_per_pixel, 1, 10);
    ImGui::InputInt("Ray bounces", &state.renderer_conf.ray_bounces, 1);
  
    ImGui::Checkbox("Reuse buffers", &state.renderer_conf.reuse_buffer);

    ImGui::Text("Tone mapping - Reinhard extended");
    ImGui::InputFloat("White point", &state.renderer_conf.white_point, 0.1f);

    if (ImGui::Button("Render"))
    {
      model.render_pressed = true;
    }
    if (state.renderer != nullptr)
    {
      if (state.renderer->is_working())
      {
        ImGui::SameLine();
        char name[50];
        std::sprintf(name, "Rendering with %s", state.renderer->get_display_name().c_str());
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), name);
      }
    
      rcpu* cpu_r = nullptr;
      if(state.renderer->is_child_of(rcpu::get_class_static()))
      {
        cpu_r = static_cast<rcpu*>(state.renderer);
      }
      if(cpu_r)
      {
        ImGui::Text("Last render time = %lld [ms]", cpu_r->get_render_time() / 1000);
        ImGui::Text("Last save time = %lld [ms]",   cpu_r->get_save_time() / 1000);
      }
#if USE_STAT
      if(cpu_r)
      {
        uint64_t rc =  cpu_r->get_ray_count();
        uint64_t rtc = cpu_r->get_ray_triangle_intersection_count();
        uint64_t rbc = cpu_r->get_ray_box_intersection_count();
        uint64_t roc = cpu_r->get_ray_object_intersection_count();
        float frc = static_cast<float>(rc);
        float frtc = static_cast<float>(rtc);
        float frbc = static_cast<float>(rbc);
        float froc = static_cast<float>(roc);
        ImGui::Text("Rays: %lld", rc);
        {
          ffpe_disabled_scope fpe;
          ImGui::Text("Ray-triangle: %lld  percentage: %f", rtc, frtc / frc);
          ImGui::Text("Ray-box: %lld percentage: %f", rbc, frbc / frc);
          ImGui::Text("Ray-object: %lld percentage: %f", roc, froc / frc);
        }
      }
#endif
    }
    else
    {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "No renderer active");
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "MATERIALS");
    ImGui::Separator();
  
    model.m_model.objects = REG.get_all_by_type<const amaterial>();
    draw_selection_combo<amaterial>(model.m_model, state, "Material", [=](const amaterial* obj) -> bool { return true; });
    if (model.m_model.selected_object != nullptr)
    {
      const_cast<amaterial*>(model.m_model.selected_object)->accept(fdraw_edit_panel());
    }
  }

  void draw_hotkeys_panel(fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CONTROLS");
    ImGui::Separator();
    ImGui::Text("F1 - Use preview renderer");
    ImGui::Text("F2 - Use reference renderer");
    ImGui::Text("F5 - Render!");
    ImGui::Text("LMB (on image) - select object");
    ImGui::Text("Scroll - Camera speed (current speed: %f)", state.camera_conf.move_speed);
    ImGui::Text("QWEASD - Camera movement");
    ImGui::Text("RMB - Camera rotation");
    ImGui::Text("ZXC + mouse - Object movement");
  }

  void draw_output_window(foutput_window_model& model, fapp_instance& state)
  {
    if (state.renderer->output_texture != nullptr)
    {
      ImGui::Begin("OUTPUT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::InputFloat("Zoom", &model.zoom, 0.1f);
      ImVec2 size = ImVec2(state.output_width * model.zoom, state.output_height * model.zoom);
      ImGui::Image((ImTextureID)state.renderer->output_srv.Get(), size, ImVec2(0, 0), ImVec2(1, 1));

      if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
      {
        ImVec2 item_min = ImGui::GetItemRectMin();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        state.output_window_lmb_x = mouse_pos.x - item_min.x;
        state.output_window_lmb_y = mouse_pos.y - item_min.y;
      }

      ImGui::Checkbox("Auto render", &model.auto_render);

      ImGui::End();
    }
  }

  void draw_scene_editor_window(fscene_editor_window_model& model, fapp_instance& state)
  {
    ImGui::Begin("SCENE", nullptr);
  
    if (ImGui::MenuItem("SAVE STATE"))
    {
      state.save_scene_state();
    }

    draw_camera_panel(model.cp_model, state);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OBJECTS");
    ImGui::Separator();

    draw_new_object_panel(model.nop_model, state);
    ImGui::SameLine();
    draw_delete_object_panel(model.d_model, state);

    int num_objects = (int)state.scene_root->objects.size();
    if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, fmath::min1(20, (float)num_objects + 1) * ImGui::GetTextLineHeightWithSpacing())))
    {
      for (int n = 0; n < num_objects; n++)
      {
        hhittable_base* obj = state.scene_root->objects[n];
        if (state.selected_object != nullptr && obj == state.selected_object)
        {
          model.m_model.selected_id = -1;
          model.selected_id = n;
          model.d_model.selected_id = n;
        }
        std::string obj_name = obj->get_display_name();
        std::ostringstream oss;
        oss << obj_name;
        if (ImGui::Selectable(oss.str().c_str(), model.selected_id == n))
        {
          model.m_model.reset();
          model.selected_id = n;
          model.d_model.selected_id = n;
          state.selected_object = nullptr;
        }
      }
      ImGui::EndListBox();
    }

    if (model.selected_id >= 0 && model.selected_id < num_objects)
    {
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SELECTED");
      ImGui::Separator();

      hhittable_base* selected_obj = state.scene_root->objects[model.selected_id];
      state.selected_object = selected_obj;
      selected_obj->accept(fdraw_edit_panel());

      model.m_model.objects = REG.get_all_by_type<const amaterial>();
      draw_selection_combo<amaterial>(model.m_model, state, "Material",
        [=](const amaterial* obj) -> bool { return true; },
        selected_obj->material_asset_ptr.get());
    
      if (model.m_model.selected_object != nullptr)
      {
        std::string selected_name = model.m_model.selected_object->file_name;
        selected_obj->material_asset_ptr.set_name(selected_name);
      }

      ImGui::Separator();
    }
    ImGui::End();
  }

  void draw_new_object_panel(fnew_object_panel_model& model, fapp_instance& state)
  {
    if (ImGui::Button("Add new"))
    {
      ImGui::OpenPopup("New object?");
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("New object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      model.c_model.objects = REG.get_classes();
      std::string hittable_class_name = hhittable_base::get_class_static()->get_class_name();

      draw_selection_combo<oclass_object>(model.c_model, state, "Class", [=](const oclass_object* obj) -> bool { return obj->get_parent_class_name() == hittable_class_name; });

      if (ImGui::Button("Add", ImVec2(120, 0)) && model.c_model.selected_object != nullptr)
      {
        state.scene_root->add(REG.spawn_from_class<hhittable_base>(model.c_model.selected_object));
        ImGui::CloseCurrentPopup();
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0)))
      {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  template<typename T>
  void draw_selection_combo(fselection_combo_model<T>& model, fapp_instance& state, const char* name, std::function<bool(const T*)> predicate, const T* default_selected_object)
  {
    if (model.objects.size() > 0)
    {
      if (model.selected_object == nullptr)
      {
        if(default_selected_object == nullptr)
        {
          model.selected_object = model.objects[0];
          model.selected_id = 0;
        }
        else
        {
          for (int i = 0; i < model.objects.size(); ++i)
          {
            if(model.objects[i] == default_selected_object)
            {
              model.selected_object = default_selected_object;
              model.selected_id = i;
              break;
            }
          }
        }
      }
      if (ImGui::BeginCombo(name, model.selected_object->get_display_name().c_str()))
      {
        for (int i = 0; i < model.objects.size(); ++i)
        {
          if (!predicate(model.objects[i]))
          {
            continue;
          }

          const bool is_selected = (model.selected_id == i);
          std::string iterated_name = model.objects[i]->get_display_name();

          if (ImGui::Selectable(iterated_name.c_str(), is_selected))
          {
            model.selected_id = i;
            model.selected_object = model.objects[i];
          }
          if (is_selected)
          {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
    }
    else
    {
      ImGui::Text("No materials to choose from");
    }
  }

  void draw_delete_object_panel(fdelete_object_panel_model& model, fapp_instance& state)
  {
    if (ImGui::Button("Delete selected"))
    {
      ImGui::OpenPopup("Delete object?");
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      hhittable_base* selected_obj = state.scene_root->objects[model.selected_id];
      if (selected_obj != nullptr)
      {
        ImGui::BeginDisabled(true);
        selected_obj->accept(fdraw_edit_panel());
        ImGui::EndDisabled();

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
          state.scene_root->remove(model.selected_id);
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndPopup();
    }
  }

  void draw_managed_objects_panel(fapp_instance& state)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "MANAGED OBJECTS");
    ImGui::Separator();

    const std::vector<oobject*>& objects = REG.get_all();

    int num_objects = (int)objects.size();
    if (ImGui::BeginListBox("Assets", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
      for (int n = 0; n < num_objects; n++)
      {
        std::ostringstream oss;
        oss << "[" << objects[n]->get_runtime_id() << "] " << objects[n]->get_display_name() << " (" << objects[n]->get_class()->get_class_name() << ")";
        if (ImGui::Selectable(oss.str().c_str()))
        {

        }
      }
      ImGui::EndListBox();
    }
  }
}