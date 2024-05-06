#include "stdafx.h"

#include "imgui.h"

#include <stdexcept>
#include <functional>
#include <DirectXMath.h>

#include "ui_helper.h"

namespace editor
{
  void fui_helper::input_float3(const char* caption, DirectX::XMFLOAT4& value)
  {
    float temp_arr[4] = { value.x, value.y, value.z, value.w };
    ImGui::InputFloat3(caption, temp_arr);
    value = { temp_arr[0], temp_arr[1], temp_arr[2], temp_arr[3] };
  }

  void fui_helper::color_edit4(const char* caption, DirectX::XMFLOAT4& value)
  {
    float temp_arr[4] = { value.x, value.y, value.z, value.w };
    ImGui::ColorEdit4(caption, temp_arr, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    value = { temp_arr[0], temp_arr[1], temp_arr[2], temp_arr[3] };
  }

  void fui_helper::check_box(const char* caption, int& value)
  {
    bool enabled = static_cast<bool>(value);
    ImGui::Checkbox(caption, &enabled);
    value = enabled;
  }

  bool fui_helper::input_text(const char* caption, std::string& text)
  {
    if (text.size() > 256)
    {
      throw std::runtime_error("Unable to edit text longer than 255 characters");
    }
    char* buffer = new char[256];
    strcpy(buffer, text.c_str());
    if (ImGui::InputText(caption, buffer, 256))
    {
      text = buffer;
      return true;
    }
    delete[] buffer;
    return false;
  }

  template<typename T>
  void fui_helper::draw_selection_combo(fselection_combo_model<T>& model, const char* name, std::function<bool(const T*)> predicate, const T* default_selected_object)
  {
    if (model.objects.size() > 0)
    {
      if (model.selected_object == nullptr)
      {
        if (default_selected_object == nullptr)
        {
          model.selected_object = model.objects[0];
          model.selected_id = 0;
        }
        else
        {
          for (int i = 0; i < model.objects.size(); ++i)
          {
            if (model.objects[i] == default_selected_object)
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
      ImGui::Text("No elements to choose from");
    }
  }

  template void fui_helper::draw_selection_combo<amaterial>(fselection_combo_model<amaterial>& model, const char* name, std::function<bool(const amaterial*)> predicate, const amaterial* default_selected_object);
  template void fui_helper::draw_selection_combo<atexture>(fselection_combo_model<atexture>& model, const char* name, std::function<bool(const atexture*)> predicate, const atexture* default_selected_object);
  template void fui_helper::draw_selection_combo<astatic_mesh>(fselection_combo_model<astatic_mesh>& model, const char* name, std::function<bool(const astatic_mesh*)> predicate, const astatic_mesh* default_selected_object);
  template void fui_helper::draw_selection_combo<oclass_object>(fselection_combo_model<oclass_object>& model, const char* name, std::function<bool(const oclass_object*)> predicate, const oclass_object* default_selected_object);
}
