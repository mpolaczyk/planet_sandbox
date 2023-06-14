#include "stdafx.h"

#include "math/materials.h"

asset_type material::get_static_asset_type() 
{ 
  return asset_type::material; 
}

material* material::load(const std::string& name)
{
  // TODO!
  // 
  // move materials to separate json files
  // parse json fiel from disk
  // read all properties of the material and spawn in using deserialzier
  
  return nullptr;
}