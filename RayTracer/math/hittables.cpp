#include "stdafx.h"

#include "math/aabb.h"
#include "math/onb.h"
#include "materials.h"
#include "mesh.h"
#include "hittables.h"

int hittable::last_id = 0;

scene::~scene()
{
  for (hittable* obj : objects)
  {
    delete obj;
  }
}


void scene::add(hittable* object)
{
  objects.push_back(object);
}

void scene::remove(int object_id) 
{ 
  delete objects[object_id]; 
  objects.erase(objects.begin() + object_id); 
}

void scene::build_boxes()
{
  LOG_TRACE("Scene: build boxes");

  assert(objects.size() > 0);
  // World collisions update
  for (hittable* object : objects)
  {
    assert(object != nullptr);
    object->get_bounding_box(object->bounding_box);
  }
}

void scene::update_materials()
{
  LOG_TRACE("Scene: update materials");

  assert(objects.size() > 0);

  // Trigger resource loading for materials.
  // Soft ptr name change may invalidate it.
  for (hittable* obj : objects)
  {
    obj->material_asset.get();
  }
}

void scene::query_lights()
{
  LOG_TRACE("Scene: query lights");

  lights_num = 0;
  for (hittable* object : objects)
  {
    const material* mat = object->material_asset.get();
    if (mat != nullptr && mat->type == material_type::light)
    {
      lights[lights_num] = object;
      lights_num++;
      assert(lights_num < MAX_LIGHTS);
    }
  }
  if (lights_num == 0)
  {
    LOG_WARN("No lights detected.");
  }
}

hittable* scene::get_random_light()
{
  assert(lights_num < MAX_LIGHTS);
  // Get next light millions of times gives the same distribution as get true random light, but is 5 times cheaper
  static int32_t last_light = 0;
  last_light = (last_light + 1) % lights_num;
  hittable* light = lights[last_light];
  assert(light != nullptr);
  return light;
}


bool sphere::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  vec3 oc = in_ray.origin - origin;
  float a = math::length_squared(in_ray.direction);
  float half_b = math::dot(oc, in_ray.direction);
  float c = math::length_squared(oc) - radius * radius;

  float delta = half_b * half_b - a * c;
  if (delta < 0.0f)
  {
    return false;
  }

  // Find the nearest root that lies in the acceptable range.
  float sqrtd = sqrt(delta);
  float root = (-half_b - sqrtd) / a;
  if (root < t_min || t_max < root)
  {
    root = (-half_b + sqrtd) / a;
    if (root < t_min || t_max < root)
    {
      return false;
    }
  }

  out_hit.t = root;
  out_hit.p = in_ray.at(out_hit.t);
  out_hit.material_ptr = material_asset.get();

  // Normal always against the ray
  vec3 outward_normal = (out_hit.p - origin) / radius;
  out_hit.front_face = math::flip_normal_if_front_face(in_ray.direction, outward_normal, out_hit.normal);
  math::get_sphere_uv(outward_normal, out_hit.u, out_hit.v);
  return true;
}

bool scene::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  float closest_so_far = t_max;

  for (hittable* object : objects)
  {
#if USE_TLAS
    if (!object->bounding_box.hit(in_ray, t_min, t_max))
    {
      continue;
    }
#endif USE_TLAS
    stats::inc_ray_object_intersection();
    if (object->hit(in_ray, t_min, closest_so_far, temp_rec))
    {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      out_hit = temp_rec;
      out_hit.object = object;
    }
  }

  return hit_anything;
}

bool xy_rect::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  if (math::is_almost_zero(in_ray.direction.z)) { return false; }
  float t = (z - in_ray.origin.z) / in_ray.direction.z;
  if (t < t_min || t > t_max) { return false; }
  float x = in_ray.origin.x + t * in_ray.direction.x;
  float y = in_ray.origin.y + t * in_ray.direction.y;
  if (x < x0 || x > x1 || y < y0 || y > y1) { return false; }
  out_hit.u = (x - x0) / (x1 - x0);
  out_hit.v = (y - y0) / (y1 - y0);
  out_hit.t = t;
  out_hit.front_face = math::flip_normal_if_front_face(in_ray.direction, vec3(0.0f, 0.0f, 1.0f), out_hit.normal);
  out_hit.material_ptr = material_asset.get();
  out_hit.p = in_ray.at(t);
  return true;
}

bool xz_rect::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  if (math::is_almost_zero(in_ray.direction.y)) { return false; }
  if (in_ray.direction.y == 0.0f) { return false; }
  float t = (y - in_ray.origin.y) / in_ray.direction.y;
  if (t < t_min || t > t_max) { return false; }
  float x = in_ray.origin.x + t * in_ray.direction.x;
  float z = in_ray.origin.z + t * in_ray.direction.z;
  if (x < x0 || x > x1 || z < z0 || z > z1) { return false; }
  out_hit.u = (x - x0) / (x1 - x0);
  out_hit.v = (z - z0) / (z1 - z0);
  out_hit.t = t;
  out_hit.front_face = math::flip_normal_if_front_face(in_ray.direction, vec3(0.0f, 1.0f, 0.0f), out_hit.normal);
  out_hit.material_ptr = material_asset.get();
  out_hit.p = in_ray.at(t);
  return true;
}

bool yz_rect::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  if (math::is_almost_zero(in_ray.direction.x)) { return false; }
  if (in_ray.direction.x == 0.0f) { return false; }
  float t = (x - in_ray.origin.x) / in_ray.direction.x;
  if (t < t_min || t > t_max) { return false; }
  float y = in_ray.origin.y + t * in_ray.direction.y;
  float z = in_ray.origin.z + t * in_ray.direction.z;
  if (y < y0 || y > y1 || z < z0 || z > z1) { return false; }
  out_hit.u = (y - y0) / (y1 - y0);
  out_hit.v = (z - z0) / (z1 - z0);
  out_hit.t = t;
  out_hit.front_face = math::flip_normal_if_front_face(in_ray.direction, vec3(1.0f, 0.0f, 0.0f), out_hit.normal);
  out_hit.material_ptr = material_asset.get();
  out_hit.p = in_ray.at(t);
  return true;
}

bool static_mesh::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  if (runtime_asset == nullptr || runtime_asset->faces.size() == 0)
  {
    return false;
  }
  const std::vector<triangle_face>& faces = runtime_asset->faces;

  bool result = false;
  hit_record best_hit;
  int hits = 0;
  bool can_refract = material_asset.get()->refraction_probability > 0.0f;
  // backface triangles can be dropped if the material is not refracting.
  // 
  // TODO: check dot product of face normal and ray direction here instead of dropping inside function
  // This required mesh with normals which does not work right now.
  
  // Multiple triangles can intersect, find the closest one.
  for (int i = 0; i < faces.size(); i++)
  {
    const triangle_face* face = &faces[i];
    hit_record h;
    if (math::ray_triangle(in_ray, t_min, t_max, face, h, !can_refract))
    {
      if (hits == 0)
      {
        best_hit = h;
        best_hit.face_id = i;
        hits++;
      }
      else if (h.t < best_hit.t)
      {
        best_hit = h;
        best_hit.face_id = i;
      }
    }
  }
  if (hits > 0)
  {
    out_hit = best_hit;
    out_hit.material_ptr = material_asset.get();
    return true;
  }

  return false;
}


vec3 sphere::get_random_point() const
{
  return vec3(random_cache::normal_distribution() * radius);
}

vec3 xy_rect::get_random_point() const
{
  float rx = random_cache::get_float_M_N(x0, x1);
  float ry = random_cache::get_float_M_N(y0, y1);
  return vec3(rx, ry, z);
}

vec3 xz_rect::get_random_point() const
{
  float rx = random_cache::get_float_M_N(x0, x1);
  float rz = random_cache::get_float_M_N(z0, z1);
  return vec3(rx, y, rz);
}

vec3 yz_rect::get_random_point() const
{
  float ry = random_cache::get_float_M_N(y0, y1);
  float rz = random_cache::get_float_M_N(z0, z1);
  return vec3(x, ry, rz);
}


float sphere::get_area() const
{
  assert(false);
  return 0.0f;
}

float xy_rect::get_area() const
{
  return (x1 - x0) * (y1 - y0);
}

float xz_rect::get_area() const
{
  return (x1 - x0) * (z1 - z0);
}

float yz_rect::get_area() const
{
  return (y1 - y0) * (z1 - z0);
}


float sphere::get_pdf_value(const vec3& look_from, const vec3& from_to) const
{
  //float cos_theta_max = sqrt(1 - radius * radius / (origin - look_from).length_squared());
  //float solid_angle = 2 * pi * (1 - cos_theta_max);
  //return  1 / solid_angle;

  // BOOK
  float cos_theta_max = sqrt(1 - radius * radius / math::length_squared(origin - look_from));
  float solid_angle = 2 * math::pi * (1 - cos_theta_max);
  
  return  1.0f / solid_angle;
}

float xy_rect::get_pdf_value(const vec3& look_from, const vec3& from_to) const
{
  hit_record rec;
  if (!hit(ray(look_from, from_to), 0.001f, math::infinity, rec))
    return 0;

  auto area = get_area();
  auto distance_squared = math::length_squared(from_to);
  auto cosine = fabs(math::dot(from_to, rec.normal) / math::length(from_to));

  return distance_squared / (cosine * area);
}

float xz_rect::get_pdf_value(const vec3& look_from, const vec3& from_to) const
{
  hit_record rec;
  if (!hit(ray(look_from, from_to), 0.001f, math::infinity, rec))
    return 0;

  auto area = get_area();
  auto distance_squared = math::length_squared(from_to);
  auto cosine = fabs(math::dot(from_to, rec.normal) / math::length(from_to));

  return distance_squared / (cosine * area);
}

float yz_rect::get_pdf_value(const vec3& look_from, const vec3& from_to) const
{
  hit_record rec;
  if (!hit(ray(look_from, from_to), 0.001f, math::infinity, rec))
    return 0;

  auto area = get_area();
  auto distance_squared = math::length_squared(from_to);
  auto cosine = fabs(math::dot(from_to, rec.normal) / math::length(from_to));

  return distance_squared / (cosine * area);
}


vec3 sphere::get_pdf_direction(const vec3& look_from) const
{
  //return random_in_unit_sphere() * radius - look_from;

  // BOOK
  vec3 direction = origin - look_from;
  auto distance_squared = math::length_squared(direction);
  onb uvw;
  uvw.build_from_w(direction);
  return uvw.local(random_cache::in_sphere(radius, distance_squared));
}

vec3 xy_rect::get_pdf_direction(const vec3& look_from) const
{
  return get_random_point() - look_from;
}

vec3 xz_rect::get_pdf_direction(const vec3& look_from) const
{
  return get_random_point() - look_from;
}

vec3 yz_rect::get_pdf_direction(const vec3& look_from) const
{
  return get_random_point() - look_from;
}


bool sphere::get_bounding_box(aabb& out_box) const
{
  out_box = aabb(origin - radius, origin + radius);
  return true;
}

bool scene::get_bounding_box(aabb& out_box) const
{
  if (objects.empty()) return false;

  aabb temp_box;
  bool first_box = true;

  for (const hittable* object : objects)
  {
    if (!object->get_bounding_box(temp_box)) return false;
    out_box = first_box ? temp_box : aabb::merge(out_box, temp_box);
    first_box = false;
  }

  return true;
}

bool xy_rect::get_bounding_box(aabb& out_box) const
{
  // The bounding box must have non-zero width in each dimension, so pad the Z
  // dimension a small amount.
  out_box = aabb(vec3(x0, y0, z - 0.0001f), vec3(x1, y1, z + 0.0001f));
  return true;
}

bool xz_rect::get_bounding_box(aabb& out_box) const
{
  // The bounding box must have non-zero width in each dimension, so pad the Y
  // dimension a small amount.
  out_box = aabb(vec3(x0, y - 0.0001f, z0), vec3(x1, y + 0.0001f, z1));
  return true;
}

bool yz_rect::get_bounding_box(aabb& out_box) const
{
  // The bounding box must have non-zero width in each dimension, so pad the X
  // dimension a small amount.
  out_box = aabb(vec3(x - 0.0001f, y0, z0), vec3(x + 0.0001f, y1, z1));
  return true;
}

bool static_mesh::get_bounding_box(aabb& out_box) const
{
  vec3 mi(0.0f);  // minimum corner
  vec3 ma(0.0f);  // maximum corner
  if (runtime_asset != nullptr)
  {
    for (const triangle_face& f : runtime_asset->faces)
    {
      const vec3& fv = f.vertices[0];
      if (fv.x < mi.x) { mi.x = fv.x; }
      if (fv.y < mi.y) { mi.y = fv.y; }
      if (fv.z < mi.z) { mi.z = fv.z; }
      if (fv.x > ma.x) { ma.x = fv.x; }
      if (fv.y > ma.y) { ma.y = fv.y; }
      if (fv.z > ma.z) { ma.z = fv.z; }
    }
  }
  out_box = aabb(mi, ma);
  return true;
}

inline uint32_t hittable::get_hash() const
{
  return hash::combine(hash::get(material_asset.get_name().c_str()), (int)type);
}

inline uint32_t sphere::get_hash() const
{
  return hash::combine(hittable::get_hash(), hash::get(origin), hash::get(radius));
}

inline uint32_t scene::get_hash() const
{
  uint32_t a = 0;
  for (const hittable* obj : objects)
  {
    a = hash::combine(a, obj->get_hash());
  }
  return a;
}

inline uint32_t xy_rect::get_hash() const
{
  uint32_t a = hash::combine(hittable::get_hash(), hash::get(x0), hash::get(y0), hash::get(x1));
  uint32_t b = hash::combine(hash::get(y1), hash::get(z));
  return hash::combine(a, b);
}

inline uint32_t xz_rect::get_hash() const
{
  uint32_t a = hash::combine(hittable::get_hash(), hash::get(x0), hash::get(z0), hash::get(x1));
  uint32_t b = hash::combine(hash::get(z1), hash::get(y));
  return hash::combine(a, b);
}

inline uint32_t yz_rect::get_hash() const
{
  uint32_t a = hash::combine(hittable::get_hash(), hash::get(y0), hash::get(z0), hash::get(y1));
  uint32_t b = hash::combine(hash::get(z1), hash::get(x));
  return hash::combine(a, b);
}

inline uint32_t static_mesh::get_hash() const
{
  uint32_t a = hash::combine(hittable::get_hash(), hash::get(origin), hash::get(extent), hash::get(rotation));
  uint32_t b = hash::combine(hash::get(scale), hash::get(material_asset.get_name().c_str()));
  return hash::combine(a, b);
}


scene* scene::clone() const
{
  scene* ans = new scene();
  *ans = *this;
  ans->objects.clear();
// Deep copy
for (const hittable* obj : objects)
{
  hittable* new_obj = obj->clone();
  ans->objects.push_back(new_obj);
}
return ans;
}

sphere* sphere::clone() const
{
  sphere* ans = new sphere();
  *ans = *this;
  return ans;
}

xy_rect* xy_rect::clone() const
{
  xy_rect* ans = new xy_rect();
  *ans = *this;
  return ans;
}

xz_rect* xz_rect::clone() const
{
  xz_rect* ans = new xz_rect();
  *ans = *this;
  return ans;
}

yz_rect* yz_rect::clone() const
{
  yz_rect* ans = new yz_rect();
  *ans = *this;
  return ans;
}

static_mesh* static_mesh::clone() const
{
  static_mesh* ans = new static_mesh();
  *ans = *this;
  return ans;
}

void hittable::load_resources()
{
  // Materials are loaded on editor startup because I need to show all of them in UI
  // This is here for consistence:
  material_asset.get();
}

void scene::load_resources()
{
  LOG_TRACE("Scene: load resources");

  assert(objects.size() > 0);
  for (hittable* object : objects)
  {
    assert(object != nullptr);
    object->load_resources();
  }
}

void static_mesh::load_resources()
{
  mesh_asset.get();
}


void scene::pre_render()
{
  LOG_TRACE("Scene: pre-render");

  assert(objects.size() > 0);
  for (hittable* object : objects)
  {
    assert(object != nullptr);
    object->pre_render();
  }
}

void static_mesh::pre_render()
{
  // Create a temporary object and modify it, reuse it next frame
  std::ostringstream oss;
  oss << "temp_" << mesh_asset.get_name() << "_hittable_id_" << id;
  runtime_asset = globals::get_asset_registry()->find_asset<mesh>(oss.str());
  if (runtime_asset == nullptr)
  {
    if (globals::get_asset_registry()->find_asset<mesh>(mesh_asset.get_name()) != nullptr)
    {
      runtime_asset = globals::get_asset_registry()->clone_asset<mesh>(mesh_asset.get()->get_runtime_id(), oss.str());
    }
    else
    {
      return;
    }
  }
  
  float y_rotation = rotation.y / 180.0f * math::pi;
  
  const std::vector<triangle_face>& mesh_faces = mesh_asset.get()->faces;
  std::vector<triangle_face>& runtime_faces = runtime_asset->faces;
  runtime_faces.clear();
  runtime_faces = mesh_faces;
  
  // Translate asset vertices to the world coordinates
  for (int f = 0; f < mesh_faces.size(); f++)
  {
    const triangle_face& in = mesh_faces[f];
    
    // Calculate world location for each vertex
    for (size_t vi = 0; vi < 3; ++vi)
    {
      runtime_faces[f].vertices[vi].x = (cosf(y_rotation) * in.vertices[vi].x - sinf(y_rotation) * in.vertices[vi].z) * scale.x + origin.x;
      runtime_faces[f].vertices[vi].y = in.vertices[vi].y * scale.x + origin.y;
      runtime_faces[f].vertices[vi].z = (sinf(y_rotation) * in.vertices[vi].x + cosf(y_rotation) * in.vertices[vi].z) * scale.x + origin.z;
    }
    
    // Vertex normals
    for (size_t ni = 0; ni < 3; ++ni)
    {
      runtime_faces[f].normals[ni].x = in.normals[ni].x;
      runtime_faces[f].normals[ni].y = in.normals[ni].y;
      runtime_faces[f].normals[ni].z = in.normals[ni].z;
    }

    //for (size_t ni = 0; ni < 3; ++ni)
    //{
    //  runtime_faces[f].normals[ni].x = (cosf(y_rotation) * in.normals[ni].x - sinf(y_rotation) * in.normals[ni].z) * scale.x + origin.x;
    //  runtime_faces[f].normals[ni].y = in.normals[ni].y * scale.x + origin.y;
    //  runtime_faces[f].normals[ni].z = (sinf(y_rotation) * in.normals[ni].x + cosf(y_rotation) * in.normals[ni].z) * scale.x + origin.z;
    //}
  }
}