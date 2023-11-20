#pragma once

#include <array>
#include <assert.h>

#include "core/core.h"

#include "math/vec3.h"
#include "math/aabb.h"
#include "math/hit.h"
#include "math/ray.h"

#include "object/object_types.h"

#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "assets/mesh.h"


namespace engine
{
  constexpr int32_t MAX_LIGHTS = 50;

  /* Warning! Adding new hittables:
  * 1. Add child class in this header.
  * 2. Add serializer and deserializer in hittables_json.h and hittables_json.cpp so that the type can be persistent.
  * 3. Extend scene_serializer::serialize and scene_serializer::deserialize so that scene knows how to serialize sub objects.
  * 4. Add draw_edit_panel and get_name in hittables_ui.cpp so that editor knows how to display UI elements for it.
  * 5. Add get_hash in hittables.cpp so that program knows when object changed.
  * 6. Add clone in hittable.cpp so that object can be cloned.
  */

  class ENGINE_API hittable
  {
  public:
    hittable() { }
    explicit hittable(const hittable* rhs) { *this = *rhs; };
    hittable(hittable_type in_type) : type(in_type)
    {

    };

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const = 0;
    virtual bool get_bounding_box(aabb& out_box) const = 0;
    virtual void get_name(std::string& out_name, bool with_params = true) const;
    virtual vec3 get_origin() const = 0;
    virtual vec3 get_extent() const = 0;
    virtual void set_origin(const vec3& value) = 0;
    virtual void set_extent(float value) = 0;
    // Deprecated begin
    virtual float get_area() const { assert(false); return 0.0f; };
    virtual float get_pdf_value(const vec3& origin, const vec3& v) const { assert(false); return 0.0f; };
    virtual vec3 get_pdf_direction(const vec3& look_from) const { assert(false); return vec3(0, 0, 0); };
    virtual vec3 get_random_point() const { assert(false); return vec3(0, 0, 0); };
    // Deprecated end

    virtual uint32_t get_hash() const;
    virtual hittable* clone() const = 0;
    virtual void load_resources();
    virtual void pre_render() {};

    // Persistent members
    hittable_type type = hittable_type::scene;
    soft_asset_ptr<material_asset> material_asset_ptr;

    // Runtime members
    aabb bounding_box;
    int id;
    static int last_id;
  };

  class ENGINE_API scene : public hittable
  {
  public:
    scene() : hittable(hittable_type::scene) {}
    explicit scene(const scene* rhs) { *this = *rhs; };
    ~scene();

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
    virtual bool get_bounding_box(aabb& out_box) const override;
    virtual void get_name(std::string& out_name, bool with_params) const override;
    virtual vec3 get_origin() const override { assert(false); return vec3(0, 0, 0); };
    virtual vec3 get_extent() const override { assert(false); return vec3(0, 0, 0); };
    virtual void set_origin(const vec3& value) override { };
    virtual void set_extent(float value) override { assert(false); };

    virtual uint32_t get_hash() const override;
    virtual scene* clone() const override;
    virtual void load_resources() override;
    virtual void pre_render() override;

    void add(hittable* object);
    void remove(int object_id);

    void build_boxes();
    void update_materials();
    void query_lights();
    hittable* get_random_light();

    // Persistent members
    std::vector<hittable*> objects;

    // Runtime members
    int32_t lights_num = 0;
    std::array<hittable*, MAX_LIGHTS> lights;
  };

  class ENGINE_API static_mesh : public hittable
  {
  public:
    static_mesh() : hittable(hittable_type::static_mesh) {}
    explicit static_mesh(const static_mesh* rhs) { *this = *rhs; };

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
    virtual bool get_bounding_box(aabb& out_box) const override;
    virtual void get_name(std::string& out_name, bool with_params) const override;
    virtual vec3 get_origin() const override { return origin; };
    virtual vec3 get_extent() const override { return vec3(extent); };
    virtual void set_origin(const vec3& value) override { origin = value; };
    virtual void set_extent(float value) override { extent = value; };

    virtual uint32_t get_hash() const override;
    virtual static_mesh* clone() const override;
    virtual void load_resources() override;
    virtual void pre_render() override;

    // Persistent state
    vec3 origin = vec3(0, 0, 0);
    vec3 scale = vec3(1, 1, 1);
    vec3 rotation = vec3(0, 0, 0);  // degrees
    soft_asset_ptr<static_mesh_asset> mesh_asset_ptr;

    // Runtime state
    static_mesh_asset* runtime_asset_ptr;  // Vertices translated to the world coordinates

    mutable float extent = 0.0f;
  };

  class ENGINE_API sphere : public hittable
  {
  public:
    sphere() : hittable(hittable_type::sphere) {}
    explicit sphere(const sphere* rhs) { *this = *rhs; };
    sphere(const vec3& in_origin, float radius) : origin(in_origin), radius(radius), hittable(hittable_type::sphere) { };

    virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
    virtual bool get_bounding_box(aabb& out_box) const override;
    virtual void get_name(std::string& out_name, bool with_params) const override;
    virtual vec3 get_origin() const override { return origin; };
    virtual vec3 get_extent() const override { return vec3(radius); };
    virtual void set_origin(const vec3& value) override { origin = value; };
    virtual void set_extent(float value) override { radius = value; };

    virtual uint32_t get_hash() const override;
    virtual sphere* clone() const override;

    // Persistent members
    vec3 origin = vec3(0, 0, 0);
    float radius = 0.0f;
  };
}