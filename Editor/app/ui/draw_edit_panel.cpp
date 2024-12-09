#include "stdafx.h"

#include "imgui.h"

#include "draw_edit_panel.h"

#include <reactphysics3d/collision/shapes/BoxShape.h>

#include "ui_helper.h"
#include "engine/physics.h"
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

    fvec3 origin = object.origin;
    fvec3 rotation = object.rotation;
    fvec3 scale = object.scale;

    ImGui::DragFloat3("Origin", origin.e);
    ImGui::DragFloat3("Rotation", rotation.e);
    ImGui::DragFloat3("Scale", scale.e);
    ImGui::Checkbox("Gravity enabled", &object.gravity_enabled);
    static const char* rigid_body_types[] = {"Static", "Kinematic", "Dynamic"};
    ImGui::Combo("Rigid body type", &object.rigid_body_type, rigid_body_types, 3);
    
    if(origin != object.origin || rotation != object.rotation || scale != object.scale)
    {
      object.transform(origin, rotation, scale);
    }
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
        object.mesh_asset_ptr.set_name(model.selected_object->name);
      }
    }

    {
      fselection_combo_model<amaterial> model;
      model.objects = REG.get_all_by_type<const amaterial>();
      fui_helper::draw_selection_combo<amaterial>(model, "Material", [=](const amaterial* obj) -> bool{ return true; }, object.material_asset_ptr.get());

      if (model.selected_object != nullptr)
      {
        object.material_asset_ptr.set_name(model.selected_object->name);
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
    ImGui::Text("File name: %s", object.name.c_str());
  }

  void vdraw_edit_panel::visit_rrenderer_base(rrenderer_base& object) const
  {
    int width = static_cast<int>(object.context.width);
    int height = static_cast<int>(object.context.height);
    ImGui::InputInt("Resolution h", &width);
    ImGui::InputInt("Resolution v", &height);
    object.context.width = width;
    object.context.height = height;
    {
      fselection_combo_model<amaterial> model;
      model.objects = REG.get_all_by_type<const amaterial>();
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
    int use_texture = object.texture_asset_ptr.get_name() != "";
    fui_helper::check_box("Use texture", use_texture);
    if (use_texture)
    {
      fselection_combo_model<atexture> model;
      model.objects = REG.get_all_by_type<const atexture>();
      fui_helper::draw_selection_combo<atexture>(model, "Texture", [=](const atexture* obj) -> bool{ return true; }, object.texture_asset_ptr.get());

      if (model.selected_object != nullptr)
      {
        object.texture_asset_ptr.set_name(model.selected_object->name);
      }
    }
    else
    {
      object.texture_asset_ptr.set_name("");
    }
  }

  void vdraw_edit_panel::visit(rgpu_forward_sync& object) const
  {
    visit_rrenderer_base(object);
  }

  void vdraw_edit_panel::visit(rgpu_deferred_sync& object) const
  {
    visit_rrenderer_base(object);
  }
}