#pragma once

#include "vec3.h"

class texture
{
public:
  virtual vec3 value(float u, float v, const vec3& p) const = 0;

  std::string id;

  std::string get_name() const;
  std::string get_id() const
  {
    return id;
  }
};

class solid_texture : public texture
{
public:
  solid_texture() {}
  explicit solid_texture(const vec3& color) : color(color) {}
  solid_texture(float r, float g, float b) : solid_texture(vec3(r, g, b)) {}

  virtual vec3 value(float u, float v, const vec3& p) const override
  {
    return color;
  }

private:
  vec3 color;
};

class checker_texture : public texture 
{
public:
  checker_texture() {}
  checker_texture(texture* even, texture* odd) : even(even), odd(odd) {}
 
  virtual vec3 value(float u, float v, const vec3& p) const override
  {
    float scale = 0.05f;
    float sines = sin(scale * p.x) * sin(scale * p.y) * sin(scale * p.z);
    return sines < 0 ? odd->value(u, v, p) : even->value(u, v, p);
  }

public:
  texture* odd = nullptr;
  texture* even = nullptr;
};

class asset_texture : public texture
{
public:
  asset_texture() {}

  virtual vec3 value(float u, float v, const vec3& p) const override
  {
    return vec3(0,0,0);
  }
};