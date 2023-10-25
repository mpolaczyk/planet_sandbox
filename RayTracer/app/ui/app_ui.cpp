#include "stdafx.h"

#include "imgui.h"

#include "app/app.h"
#include "processing/chunk_generator.h"
#include "processing/async_renderer_base.h"
#include "engine/camera.h"
#include "math/materials.h"
#include "app/factories.h"

void draw_raytracer_window(raytracer_window_model& model, app_instance& state)
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
  draw_asset_registry_panel(state);
  ImGui::End();
}

void draw_camera_panel(camera_panel_model& model, app_instance& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAMERA");
  ImGui::Separator();
  float ar[2] = { state.camera_conf->aspect_ratio_w, state.camera_conf->aspect_ratio_h };
  ImGui::InputFloat2("Aspect ratio", ar);
  state.camera_conf->aspect_ratio_w = ar[0];
  state.camera_conf->aspect_ratio_h = ar[1];
  ImGui::Text("Aspect ratio = %.3f", state.camera_conf->aspect_ratio_w / state.camera_conf->aspect_ratio_h);
  ImGui::InputFloat("Field of view", &state.camera_conf->field_of_view, 1.0f, 189.0f, "%.0f");
  ImGui::InputFloat("Projection", &state.camera_conf->type, 0.1f, 1.0f, "%.2f");
  ImGui::Text("0 = Perspective; 1 = Orthografic");
  ImGui::Separator();
  ImGui::InputFloat3("Look from", state.camera_conf->look_from.e, "%.2f");
  ImGui::InputFloat3("Look direction", state.camera_conf->look_dir.e, "%.2f");
  ImGui::Separator();
  ImGui::InputFloat("Focus distance", &state.camera_conf->dist_to_focus, 0.0f, 1000.0f, "%.2f");
  ImGui::InputFloat("Aperture", &state.camera_conf->aperture, 0.1f, 1.0f, "%.2f");
}

void draw_renderer_panel(renderer_panel_model& model, app_instance& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RENDERER");
  ImGui::Separator();
  ImGui::InputInt("Resolution v", &state.renderer_conf->resolution_vertical, 1, 2160);
  state.renderer_conf->resolution_horizontal = (int)((float)state.renderer_conf->resolution_vertical * state.camera_conf->aspect_ratio_w / state.camera_conf->aspect_ratio_h);
  ImGui::Text("Resolution h = %d", state.renderer_conf->resolution_horizontal);

  ImGui::Separator();
  int renderer = (int)state.renderer_conf->type;
  ImGui::Combo("Renderer", &renderer, renderer_type_names, IM_ARRAYSIZE(renderer_type_names));
  state.renderer_conf->type = (renderer_type)renderer;
  ImGui::InputInt("Rays per pixel", &state.renderer_conf->rays_per_pixel, 1, 10);
  ImGui::InputInt("Ray bounces", &state.renderer_conf->ray_bounces, 1);
  
  ImGui::Checkbox("Reuse buffers", &state.renderer_conf->reuse_buffer);

  ImGui::Text("Tone mapping - Reinhard extended");
  ImGui::InputFloat("White point", &state.renderer_conf->white_point, 0.1f);

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
      std::sprintf(name, "Rendering with %s renderer", state.renderer->get_name().c_str());
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), name);
    }
    ImGui::Text("Last render time = %lld [ms]", state.renderer->get_render_time() / 1000);
    ImGui::Text("Last save time = %lld [ms]", state.renderer->get_save_time() / 1000);
#if USE_STAT
    uint64_t rc = state.renderer->get_ray_count();
    uint64_t rtc = state.renderer->get_ray_triangle_intersection_count();
    uint64_t rbc = state.renderer->get_ray_box_intersection_count();
    uint64_t roc = state.renderer->get_ray_object_intersection_count();
    float frc = static_cast<float>(rc);
    float frtc = static_cast<float>(rtc);
    float frbc = static_cast<float>(rbc);
    float froc = static_cast<float>(roc);
    ImGui::Text("Rays: %lld", rc);
    {
      fpe_disabled_scope fpe;
      ImGui::Text("Ray-triangle: %lld  percentage: %f", rtc, frtc / frc);
      ImGui::Text("Ray-box: %lld percentage: %f", rbc, frbc / frc);
      ImGui::Text("Ray-object: %lld percentage: %f", roc, froc / frc);
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

  draw_material_selection_combo(model.m_model, state);

  material* mat = globals::get_asset_registry()->get_asset<material>(model.m_model.selected_material_id);
  if (mat != nullptr)
  {
    mat->draw_edit_panel();
  }
}

void draw_hotkeys_panel(app_instance& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CONTROLS");
  ImGui::Separator();
  ImGui::Text("F1 - Use preview renderer");
  ImGui::Text("F2 - Use reference renderer");
  ImGui::Text("F5 - Render!");
  ImGui::Text("LMB (on image) - select object");
  ImGui::Text("Scroll - Camera speed (current speed: %f)", state.move_speed);
  ImGui::Text("QWEASD - Camera movement");
  ImGui::Text("RMB - Camera rotation");
  ImGui::Text("ZXC + mouse - Object movement");
}

void draw_output_window(output_window_model& model, app_instance& state)
{
  if (state.output_texture != nullptr)
  {
    ImGui::Begin("OUTPUT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::InputFloat("Zoom", &model.zoom, 0.1f);
    ImVec2 size = ImVec2(state.output_width * model.zoom, state.output_height * model.zoom);
    ImGui::Image((ImTextureID)state.output_srv, size, ImVec2(0, 1), ImVec2(1, 0));

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
      ImVec2 itemMin = ImGui::GetItemRectMin();
      ImVec2 mousePos = ImGui::GetMousePos();
      state.output_window_lmb_x = mousePos.x - itemMin.x;
      state.output_window_lmb_y = mousePos.y - itemMin.y;
    }

    ImGui::Checkbox("Auto render on scene change", &model.auto_render);

    ImGui::End();
  }
}

void draw_scene_editor_window(scene_editor_window_model& model, app_instance& state)
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
  if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, math::min1(20, (float)num_objects + 1) * ImGui::GetTextLineHeightWithSpacing())))
  {
    for (int n = 0; n < num_objects; n++)
    {
      hittable* obj = state.scene_root->objects[n];
      if (state.selected_object != nullptr && obj == state.selected_object)
      {
        model.m_model.selected_material_id = -1;
        model.selected_id = n;
        model.d_model.selected_id = n;
      }
      std::string obj_name;
      obj->get_name(obj_name, true);
      std::ostringstream oss;
      oss << obj_name;
      if (ImGui::Selectable(oss.str().c_str(), model.selected_id == n))
      {
         model.m_model.selected_material_id = -1;
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

    hittable* selected_obj = state.scene_root->objects[model.selected_id];
    state.selected_object = selected_obj;
    selected_obj->draw_edit_panel();

    material_selection_combo_model m_model;
    if (model.m_model.selected_material_id == -1)
    {
      const material* mat = selected_obj->material_asset.get();
      if (mat != nullptr)
      {
        m_model.selected_material_id = mat->get_runtime_id();
      }
    }
    draw_material_selection_combo(m_model, state);
    if (m_model.selected_material_id != -1)
    {
      std::string selected_name = globals::get_asset_registry()->get_name(m_model.selected_material_id);
      selected_obj->material_asset.set_name(selected_name);
    }

    ImGui::Separator();
  }
  ImGui::End();
}

void draw_new_object_panel(new_object_panel_model& model, app_instance& state)
{
  if (ImGui::Button("Add new"))
  {
    ImGui::OpenPopup("New object?");
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("New object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::Combo("Object type", &model.selected_type, hittable_type_names, IM_ARRAYSIZE(hittable_type_names));
    ImGui::Separator();

    if (model.hittable != nullptr && (int)model.hittable->type != model.selected_type)
    {
      delete model.hittable; // Object type changed, destroy the old one
      model.hittable = nullptr;
    }
    if (model.hittable == nullptr)
    {
      // New object
      model.hittable = object_factory::spawn_hittable((hittable_type)model.selected_type);
      model.hittable->set_origin(state.center_of_scene);
    }

    if (model.hittable != nullptr)
    {
      model.hittable->draw_edit_panel();

      draw_material_selection_combo(model.m_model, state);
    }

    if (ImGui::Button("Add", ImVec2(120, 0)) && model.hittable != nullptr)
    {
      model.hittable->material_asset.set_name(globals::get_asset_registry()->get_name(model.m_model.selected_material_id));
      state.scene_root->add(model.hittable);
      model.hittable = nullptr;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      if (model.hittable)
      {
        delete model.hittable;
        model.hittable = nullptr;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void draw_material_selection_combo(material_selection_combo_model& model, app_instance& state)
{
  std::vector<material*> materials = globals::get_asset_registry()->get_assets<material>();
  material* selected_material = globals::get_asset_registry()->get_asset<material>(model.selected_material_id);

  if (materials.size() > 0)
  {
    if (selected_material == nullptr)
    {
      selected_material = materials[0];
      model.selected_material_id = selected_material->get_runtime_id();
    }
    if (ImGui::BeginCombo("Material", selected_material->get_asset_name().c_str()))
    {
      for (int i = 0; i < materials.size(); ++i)
      {
        int iterated_id = materials[i]->get_runtime_id();
        std::string iterated_name = materials[i]->get_asset_name();

        const bool isSelected = (model.selected_material_id == iterated_id);
        if (ImGui::Selectable(iterated_name.c_str(), isSelected))
        {
          model.selected_material_id = iterated_id;
        }
        if (isSelected)
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

void draw_delete_object_panel(delete_object_panel_model& model, app_instance& state)
{
  if (ImGui::Button("Delete selected"))
  {
    ImGui::OpenPopup("Delete object?");
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("Delete object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    hittable* selected_obj = state.scene_root->objects[model.selected_id];
    if (selected_obj != nullptr)
    {
      ImGui::BeginDisabled(true);
      selected_obj->draw_edit_panel();
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

void draw_asset_registry_panel(app_instance& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ASSET REGISTRY");
  ImGui::Separator();

  const std::vector<asset*>& assets = globals::get_asset_registry()->get_all_assets();

  int num_objects = (int)assets.size();
  if (ImGui::BeginListBox("Assets", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
  {
    for (int n = 0; n < num_objects; n++)
    {
      if (ImGui::Selectable(assets[n]->get_display_name().c_str()))
      {
        
      }
    }
    ImGui::EndListBox();
  }

  
}