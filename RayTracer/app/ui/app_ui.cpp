#include "stdafx.h"

#include "imgui.h"

#include "app/app.h"
#include "math/chunk_generator.h"
#include "renderer/cpu_renderer_base.h"
#include "math/camera.h"

#include "core/core.h"
#include "engine.h"
#include "hittables/hittables.h"
#include "hittables/static_mesh.h"
#include "hittables/scene.h"


void material_asset_draw_edit_panel(material_asset* obj)
{
  ImGui::Checkbox("Is light", &obj->is_light);

  ImGui::ColorEdit3("Color", obj->color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::ColorEdit3("Emitted color", obj->emitted_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

  ImGui::DragFloat("Smoothness", &obj->smoothness, 0.01f, 0.0f, 1.0f);

  ImGui::DragFloat("Gloss probability", &obj->gloss_probability, 0.01f, 0.0f, 1.0f);
  ImGui::ColorEdit3("Gloss color", obj->gloss_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

  ImGui::DragFloat("Refraction probability", &obj->refraction_probability, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat("Refraction index", &obj->refraction_index, 0.01f);
}


// FIX implement the visitor panel for hittables, use RTTI to call proper function, sphere is missing
void hittable_draw_edit_panel(hittable* obj)
{
  std::string hittable_name = obj->get_display_name();
  ImGui::Text("Object: ");
  ImGui::SameLine();
  ImGui::Text(hittable_name.c_str());
}

void scene_draw_edit_panel(scene* obj)
{
  hittable_draw_edit_panel(obj);
}

void static_mesh_draw_edit_panel(static_mesh* obj)
{
  hittable_draw_edit_panel(obj);
  ImGui::DragFloat3("Origin", obj->origin.e);
  ImGui::DragFloat3("Rotation", obj->rotation.e);
  ImGui::DragFloat3("Scale", obj->scale.e);
  {
    std::string name = obj->mesh_asset_ptr.get_name();
    assert(name.size() <= 256);
    char* buffer = new char[256];
    strcpy(buffer, name.c_str());
    if (ImGui::InputText("Object file", buffer, 256))
    {
      obj->mesh_asset_ptr.set_name(buffer);
    }
    delete[] buffer;
  }
}


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
  draw_managed_objects_panel(state);
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

  draw_renderer_selection_combo(model.r_model, state);

  /*int renderer = (int)state.renderer_conf->type;
  ImGui::Combo("Renderer", &renderer, object_type_names, IM_ARRAYSIZE(object_type_names));
  state.renderer_conf->type = static_cast<object_type>(renderer);*/

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

  engine::material_asset* mat = engine::REG.get<engine::material_asset>(model.m_model.selected_id);
  if (mat != nullptr)
  {
    material_asset_draw_edit_panel(mat);
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
        model.m_model.selected_id = -1;
        model.selected_id = n;
        model.d_model.selected_id = n;
      }
      std::string obj_name = obj->get_display_name();
      std::ostringstream oss;
      oss << obj_name;
      if (ImGui::Selectable(oss.str().c_str(), model.selected_id == n))
      {
         model.m_model.selected_id = -1;
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
    hittable_draw_edit_panel(selected_obj);

    material_selection_combo_model m_model;
    if (model.m_model.selected_id == -1)
    {
      const engine::material_asset* mat = selected_obj->material_asset_ptr.get();
      if (mat != nullptr)
      {
        m_model.selected_id = mat->get_runtime_id();
      }
    }
    draw_material_selection_combo(m_model, state);
    if (m_model.selected_id != -1)
    {
      std::string selected_name = engine::REG.get<engine::material_asset>(m_model.selected_id)->file_name;
      selected_obj->material_asset_ptr.set_name(selected_name);
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
    model.c_model.objects = REG.get_classes();
    std::string hittable_class_name = hittable::get_class_static()->class_name;

    draw_selection_combo<class_object>(model.c_model, state, [=](const class_object* obj) -> bool { return obj->parent_class_name == hittable_class_name; });

    if (ImGui::Button("Add", ImVec2(120, 0)) && model.c_model.selected_object != nullptr)
    {
      state.scene_root->add(model.c_model.selected_object->spawn_instance<hittable>());
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
void draw_selection_combo(selection_combo_model<T>& model, app_instance& state, std::function<bool(const T*)> predicate)
{
  if (model.objects.size() > 0)
  {
    if (model.selected_object == nullptr)
    {
      model.selected_object = model.objects[0];
      model.selected_id = 0;
    }
    if (ImGui::BeginCombo("Class", model.selected_object->get_display_name().c_str()))
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

void draw_hittable_selection_combo(hittable_selection_combo_model& model, app_instance& state)
{
  using namespace engine;
  std::vector<hittable*> hittables = REG.get_all_by_type<hittable>();
  hittable* selected_hittable = REG.get<hittable>(model.selected_id);

  if (hittables.size() > 0)
  {
    if (selected_hittable == nullptr)
    {
      selected_hittable = hittables[0];
      model.selected_id = selected_hittable->get_runtime_id();
    }
    if (ImGui::BeginCombo("Material", selected_hittable->get_display_name().c_str()))
    {
      for (int i = 0; i < hittables.size(); ++i)
      {
        int iterated_id = hittables[i]->get_runtime_id();
        std::string iterated_name = hittables[i]->get_display_name();

        const bool isSelected = (model.selected_id == iterated_id);
        if (ImGui::Selectable(iterated_name.c_str(), isSelected))
        {
          model.selected_id = iterated_id;
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

void draw_material_selection_combo(material_selection_combo_model& model, app_instance& state)
{
  std::vector<engine::material_asset*> materials = engine::REG.get_all_by_type<engine::material_asset>();
  engine::material_asset* selected_material = engine::REG.get<engine::material_asset>(model.selected_id);

  if (materials.size() > 0)
  {
    if (selected_material == nullptr)
    {
      selected_material = materials[0];
      model.selected_id = selected_material->get_runtime_id();
    }
    if (ImGui::BeginCombo("Material", selected_material->get_display_name().c_str()))
    {
      for (int i = 0; i < materials.size(); ++i)
      {
        int iterated_id = materials[i]->get_runtime_id();
        std::string iterated_name = materials[i]->get_display_name();

        const bool isSelected = (model.selected_id == iterated_id);
        if (ImGui::Selectable(iterated_name.c_str(), isSelected))
        {
          model.selected_id = iterated_id;
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

void draw_renderer_selection_combo(renderer_selection_combo_model& model, app_instance& state)
{
  std::vector<engine::cpu_renderer_base*> renderers = engine::REG.get_all_by_type<engine::cpu_renderer_base>();
  engine::cpu_renderer_base* selected_renderer = engine::REG.get<engine::cpu_renderer_base>(model.selected_id);

  if (renderers.size() > 0)
  {
    if (selected_renderer == nullptr)
    {
      selected_renderer = renderers[0];
      model.selected_id = selected_renderer->get_runtime_id();
    }
    if (ImGui::BeginCombo("Renderer", selected_renderer->get_name().c_str()))
    {
      for (int i = 0; i < renderers.size(); ++i)
      {
        int iterated_id = renderers[i]->get_runtime_id();
        std::string iterated_name = renderers[i]->get_name();

        const bool isSelected = (model.selected_id == iterated_id);
        if (ImGui::Selectable(iterated_name.c_str(), isSelected))
        {
          model.selected_id = iterated_id;
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
      hittable_draw_edit_panel(selected_obj);
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

void draw_managed_objects_panel(app_instance& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "MANAGED OBJECTS");
  ImGui::Separator();

  const std::vector<object*>& objects = engine::REG.get_all();

  int num_objects = (int)objects.size();
  if (ImGui::BeginListBox("Assets", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
  {
    for (int n = 0; n < num_objects; n++)
    {
      std::ostringstream oss;
      oss << "[" << objects[n]->get_runtime_id() << "] " << objects[n]->get_display_name() << " (" << objects[n]->get_class()->class_name << ")";
      if (ImGui::Selectable(oss.str().c_str()))
      {

      }
    }
    ImGui::EndListBox();
  }

  
}