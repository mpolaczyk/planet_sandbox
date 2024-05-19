#include "stdafx.h"

#include "imgui.h"

#include "draw_edit_panel.h"

#include "ui_helper.h"
#include "hittables/hittables.h"
#include "hittables/light.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"

#include "renderers/gpu_forward_sync.h"
#include "renderers/gpu_deferred_sync.h"

namespace editor
{
  void vdraw_edit_panel::visit_hhittable_base(hhittable_base& object) const
  {
    std::string display_name = object.get_display_name();
    if (fui_helper::input_text("Name", display_name))
    {
      object.set_display_name(display_name);
    }

    ImGui::DragFloat3("Origin", object.origin.e);
    ImGui::DragFloat3("Rotation", object.rotation.e);
    ImGui::DragFloat3("Scale", object.scale.e);
  }

  void vdraw_edit_panel::visit(hstatic_mesh& object) const
  {
    visit_hhittable_base(object);

    {
      fselection_combo_model<astatic_mesh> model;
      model.objects = REG.get_all_by_type<const astatic_mesh>();
      fui_helper::draw_selection_combo<astatic_mesh>(model, "Mesh", [=](const astatic_mesh* obj) -> bool{ return true; }, object.mesh_asset_ptr.get());

      if (model.selected_object != nullptr)
      {
        object.mesh_asset_ptr.set_name(model.selected_object->file_name);
      }
    }

    {
      fselection_combo_model<amaterial> model;
      model.objects = REG.get_all_by_type<const amaterial>();
      fui_helper::draw_selection_combo<amaterial>(model, "Material", [=](const amaterial* obj) -> bool{ return true; }, object.material_asset_ptr.get());

      if (model.selected_object != nullptr)
      {
        object.material_asset_ptr.set_name(model.selected_object->file_name);
      }
    }
  }

  void vdraw_edit_panel::visit(hsphere& object) const
  {
    visit_hhittable_base(object);
    ImGui::InputFloat("Radius", &object.radius);
  }

  void vdraw_edit_panel::visit(hlight& object) const
  {
    std::string display_name = object.get_display_name();
    if (fui_helper::input_text("Name", display_name))
    {
      object.set_display_name(display_name);
    }
    ImGui::DragFloat3("Origin", object.origin.e);
    fui_helper::check_box("Enabled", object.properties.enabled);
    fui_helper::color_edit4("Color", object.properties.color);
    ImGui::SliderInt("Light type", &object.properties.light_type, 0, 2);
    ImGui::Text("0 - Point");
    ImGui::Text("1 - Directional");
    fui_helper::input_float3("Direction", object.properties.direction);
    ImGui::Text("2 - Spot");
    ImGui::DragFloat("Angle", &object.properties.spot_angle, 0.01f, 0.0f, 3.15f);
    ImGui::Text("Attenuation:");
    ImGui::DragFloat("Constant", &object.properties.constant_attenuation, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Linear", &object.properties.linear_attenuation, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Quadratic", &object.properties.quadratic_attenuation, 0.01f, 0.0f, 1.0f);
  }

  void vdraw_edit_panel::visit_aasset_base(aasset_base& object) const
  {
    ImGui::Text("File name: %s", object.file_name.c_str());
  }

  void vdraw_edit_panel::visit_rrenderer_base(rrenderer_base& object) const
  {
    ImGui::InputInt("Resolution h", &object.output_width);
    ImGui::InputInt("Resolution v", &object.output_height);
    {
      fselection_combo_model<amaterial> model;
      model.objects = REG.get_all_by_type<const amaterial>();
      fui_helper::draw_selection_combo<amaterial>(model, "Default material", [=](const amaterial* obj) -> bool{ return true; }, object.default_material_asset.get());

      if (model.selected_object != nullptr)
      {
        object.default_material_asset.set_name(model.selected_object->file_name);
      }
    }
  }

  void vdraw_edit_panel::visit(amaterial& object) const
  {
    visit_aasset_base(object);
    fui_helper::color_edit4("Emissive", object.properties.emissive);
    fui_helper::color_edit4("Ambient", object.properties.ambient);
    fui_helper::color_edit4("Diffuse", object.properties.diffuse);
    fui_helper::color_edit4("Specular", object.properties.specular);
    ImGui::DragFloat("Specular power", &object.properties.specular_power, 0.1f, 1.0f, 100.0f);
    fui_helper::check_box("Use texture", object.properties.use_texture);
    if (object.properties.use_texture)
    {
      fselection_combo_model<atexture> model;
      model.objects = REG.get_all_by_type<const atexture>();
      fui_helper::draw_selection_combo<atexture>(model, "Texture", [=](const atexture* obj) -> bool{ return true; }, object.texture_asset_ptr.get());

      if (model.selected_object != nullptr)
      {
        object.texture_asset_ptr.set_name(model.selected_object->file_name);
      }
    }
  }

  void vdraw_edit_panel::visit(rgpu_forward_sync& object) const
  {
    visit_rrenderer_base(object);
    {
      fselection_combo_model<apixel_shader> model;
      model.objects = REG.get_all_by_type<const apixel_shader>();
      fui_helper::draw_selection_combo<apixel_shader>(model, "Pixel shader", [=](const apixel_shader* obj) -> bool{ return true; }, object.pixel_shader_asset.get());

      if (model.selected_object != nullptr)
      {
        object.pixel_shader_asset.set_name(model.selected_object->file_name);
      }
    }
    {
      fselection_combo_model<avertex_shader> model;
      model.objects = REG.get_all_by_type<const avertex_shader>();
      fui_helper::draw_selection_combo<avertex_shader>(model, "Vertex shader", [=](const avertex_shader* obj) -> bool{ return true; }, object.vertex_shader_asset.get());

      if (model.selected_object != nullptr)
      {
        object.vertex_shader_asset.set_name(model.selected_object->file_name);
      }
    }

    fui_helper::check_box("Show emissive", object.show_emissive);
    fui_helper::check_box("Show ambient", object.show_ambient);
    fui_helper::check_box("Show diffuse", object.show_diffuse);
    fui_helper::check_box("Show specular", object.show_specular);
    fui_helper::check_box("Show normals", object.show_normals);
    fui_helper::check_box("Show object id", object.show_object_id);
  }

  void vdraw_edit_panel::visit(rgpu_deferred_sync& object) const
  {
    visit_rrenderer_base(object);
    {
      fselection_combo_model<apixel_shader> model;
      model.objects = REG.get_all_by_type<const apixel_shader>();
      fui_helper::draw_selection_combo<apixel_shader>(model, "GBuffer pixel shader", [=](const apixel_shader* obj) -> bool{ return true; }, object.gbuffer_pixel_shader_asset.get());

      if (model.selected_object != nullptr)
      {
        object.gbuffer_pixel_shader_asset.set_name(model.selected_object->file_name);
      }
    }
    {
      fselection_combo_model<avertex_shader> model;
      model.objects = REG.get_all_by_type<const avertex_shader>();
      fui_helper::draw_selection_combo<avertex_shader>(model, "GBuffer vertex shader", [=](const avertex_shader* obj) -> bool{ return true; }, object.gbuffer_vertex_shader_asset.get());

      if (model.selected_object != nullptr)
      {
        object.gbuffer_vertex_shader_asset.set_name(model.selected_object->file_name);
      }
    }
  }
}