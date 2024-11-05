#pragma once

#include "core/core.h"
#include "asset/soft_asset_ptr.h"
#include "assets/material.h"

namespace engine
{
  class fwindow;
  class hscene;
  class hhittable_base;
  struct fshader_render_state;
  struct fscene_acceleration;
  struct fdescriptor_heap;
  
  struct ENGINE_API frenderer_context
  {    
    // Only runtime members!
    const hhittable_base* selected_object = nullptr;          // weak ptr
    hscene* scene = nullptr;                                  // weak ptr
    uint32_t back_buffer_index = 0;
    uint32_t back_buffer_count = 2;
    fdescriptor_heap* main_descriptor_heap = nullptr; // srv, cbv, uav
    ComPtr<ID3D12Resource> rtv;
    ComPtr<ID3D12Resource> dsv;

    bool validate() const
    {
      return scene != nullptr
        && back_buffer_count > 0
        && back_buffer_index >= 0 && back_buffer_index < back_buffer_count
        && main_descriptor_heap != nullptr
        && rtv != nullptr
        && dsv != nullptr;
    }
  };
}
