#pragma once

#include <string>

#include "core/core.h"

namespace engine
{
  class soft_asset_ptr_base_serializer;

  struct ENGINE_API soft_asset_ptr_base
  {
    friend class soft_asset_ptr_base_serializer;

  protected:
    // Persistent name, or the one used to discovery on the disk
    // Can't change at runtime as I have no dependency update mechanism
    std::string name;
  };

  // Soft asset pointer - persistent weak sync loading pointer to an asset
  // First get() call will trigger sync load and register asset
  // Second get() call will return cached pointer
  // No ref counting, no ownership
  // set_name can be called multiple times with different values, this will invalidate existing pointer and load different asset
  template<typename T>
  struct ENGINE_API soft_asset_ptr : public soft_asset_ptr_base
  {
    void set_name(const std::string& in_name);

    std::string get_name() const;

    bool is_loaded() const;

    const T* get() const;

  private:

    mutable T* object = nullptr;
  };
}