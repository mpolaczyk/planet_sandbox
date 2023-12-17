#pragma once

#include <string>

#include "core/core.h"
#include "core/concepts.h"

namespace engine
{
  class asset_base;
  class persistence;
  
  struct ENGINE_API soft_asset_ptr_base
  {
    friend class persistence;
    
  protected:
    // Persistent name, or the one used to discovery on the disk
    // Can't change at runtime as I have no dependency update mechanism
    std::string name;
  };

  // Soft asset pointer - persistent weak sync loading pointer to an asset_base
  // First get() call will trigger sync load and register asset
  // Second get() call will return cached pointer
  // No ref counting, no ownership
  // set_name can be called multiple times with different values, this will invalidate existing pointer and load different asset
  template<derives_from<asset_base> T>
  struct ENGINE_API soft_asset_ptr : public soft_asset_ptr_base
  {
    void set_name(const std::string& in_name);

    std::string get_name() const;

    bool is_loaded() const;

    const T* get() const;

  private:

    mutable T* asset_ptr = nullptr;
  };
}