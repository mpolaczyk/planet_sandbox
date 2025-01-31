#pragma once

#include <string>

#include "core/core.h"

namespace engine
{
  class aasset_base;
  class fpersistence;

  struct ENGINE_API fsoft_asset_ptr_base
  {
    friend class fpersistence;

  protected:
    // Persistent name, or the one used to discovery on the disk
    std::string name;
  };

  // Soft asset pointer - persistent weak sync loading pointer to an asset_base
  // First get() call will trigger sync load and register asset
  // Second get() call will return cached pointer
  // No ref counting, no ownership
  // set_name can be called multiple times with different values, this will invalidate existing pointer and load different asset
  template <typename T>
  struct ENGINE_API fsoft_asset_ptr : public fsoft_asset_ptr_base
  {
    fsoft_asset_ptr(const std::string& in_name);
    CTOR_DEFAULT(fsoft_asset_ptr)
    CTOR_MOVE_COPY_DEFAULT(fsoft_asset_ptr)
    DTOR_DEFAULT(fsoft_asset_ptr)
    
    void set_name(const std::string& in_name);
    std::string get_name() const;

    bool is_loaded() const;
    bool is_set() const;

    const T* get() const;
    T* get();

  private:
    mutable T* asset_ptr = nullptr;
  };
}
