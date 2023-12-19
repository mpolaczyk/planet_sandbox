﻿#include "stdafx.h"

#include "imgui.h"

#include "draw_edit_panel.h"

#include "hittables/hittables.h"
#include "hittables/scene.h"
#include "hittables/sphere.h"
#include "hittables/static_mesh.h"

void draw_edit_panel::visit(class hittable& object) const
{
  std::string hittable_name = object.get_display_name();
  ImGui::Text("Object: ");
  ImGui::SameLine();
  ImGui::Text(hittable_name.c_str());
}

void draw_edit_panel::visit(class scene& object) const
{
  object.hittable::accept(draw_edit_panel());
}

void draw_edit_panel::visit(class static_mesh& object) const 
{
  object.hittable::accept(draw_edit_panel());
  ImGui::DragFloat3("Origin", object.origin.e);
  ImGui::DragFloat3("Rotation", object.rotation.e);
  ImGui::DragFloat3("Scale", object.scale.e);
  {
    std::string name = object.mesh_asset_ptr.get_name();
    assert(name.size() <= 256);
    char* buffer = new char[256];
    strcpy(buffer, name.c_str());
    if (ImGui::InputText("Object file", buffer, 256))
    {
      object.mesh_asset_ptr.set_name(buffer);
    }
    delete[] buffer;
  }
}

void draw_edit_panel::visit(class sphere& object) const
{
  object.hittable::accept(draw_edit_panel());
  ImGui::DragFloat3("Origin", object.origin.e);
  ImGui::InputFloat("Radius", &object.radius);
}

void draw_edit_panel::visit(class material_asset& object) const
{
  ImGui::Checkbox("Is light", &object.is_light);

  ImGui::ColorEdit3("Color", object.color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::ColorEdit3("Emitted color", object.emitted_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

  ImGui::DragFloat("Smoothness", &object.smoothness, 0.01f, 0.0f, 1.0f);

  ImGui::DragFloat("Gloss probability", &object.gloss_probability, 0.01f, 0.0f, 1.0f);
  ImGui::ColorEdit3("Gloss color", object.gloss_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);

  ImGui::DragFloat("Refraction probability", &object.refraction_probability, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat("Refraction index", &object.refraction_index, 0.01f);
}