#include "stdafx.h"

#include <filesystem>
#include <random>
#include <fstream>

#include "gfx/stb_image.h"
#include "gfx/tiny_obj_loader.h"

#include "textures.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "common.h"

namespace stats
{
  static std::atomic<uint64_t> rays{};
  static std::atomic<uint64_t> ray_triangle_intersection{};
  static std::atomic<uint64_t> ray_box_intersection{};
  static std::atomic<uint64_t> ray_object_intersection{};

  void reset()
  {
    rays = 0;
    ray_box_intersection = 0;
    ray_triangle_intersection = 0;
    ray_object_intersection = 0;
  }
  void inc_ray()
  {
#if USE_STAT
    rays++;
#endif
  }
  void inc_ray_triangle_intersection()
  {
#if USE_STAT
    ray_triangle_intersection++;
#endif
  }
  void inc_ray_box_intersection()
  {
#if USE_STAT
    ray_box_intersection++;
#endif
  }
  void inc_ray_object_intersection()
  {
#if USE_STAT
    ray_object_intersection++;
#endif
  }
  uint64_t get_ray_count()
  {
    return rays;
  }
  uint64_t get_ray_triangle_intersection_count()
  {
    return ray_triangle_intersection;
  }
  uint64_t get_ray_box_intersection_count()
  {
    return ray_box_intersection;
  }
  uint64_t get_ray_object_intersection_count()
  {
    return ray_object_intersection;
  }
}

namespace math
{
  float reflectance(float cosine, float ref_idx)
  {
    // Use Schlick's approximation for reflectance.
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
  }

  bool flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal)
  {
    if (dot(in_ray_direction, in_outward_normal) < 0)
    {
      // Ray is inside
      out_normal = in_outward_normal;
      return true;
    }
    else
    {
      // Ray is outside
      out_normal = -in_outward_normal;
      return false;
    }
  }

  vec3 reflect(const vec3& vec, const vec3& normal)
  {
    return vec - 2 * dot(vec, normal) * normal;
  }

  vec3 refract(const vec3& v, const vec3& n, float etai_over_etat)
  {
    float cos_theta = fmin(dot(-v, n), 1.0f);
    vec3 r_out_perpendicular = etai_over_etat * (v + cos_theta * n);
    vec3 r_out_parallel = -sqrt(fabs(1.0f - math::length_squared(r_out_perpendicular))) * n;
    return r_out_perpendicular + r_out_parallel;
  }

  vec3 lerp_vec3(const vec3& a, const vec3& b, float f)
  {
    return vec3(math::lerp_float(a.x, b.x, f), math::lerp_float(a.y, b.y, f), math::lerp_float(a.z, b.z, f));
  }

  vec3 clamp_vec3(float a, float b, const vec3& f)
  {
    vec3 ans;
    ans.x = math::clamp(f.x, a, b);
    ans.y = math::clamp(f.y, a, b);
    ans.z = math::clamp(f.z, a, b);
    return ans;
  }

  bool ray_triangle(const ray& in_ray, float t_min, float t_max, const triangle_face* in_triangle, hit_record& out_hit, bool drop_backface)
  {
    assert(in_triangle != nullptr);
    stats::inc_ray_triangle_intersection();
    using namespace math;

    // https://graphicscodex.courses.nvidia.com/app.html?page=_rn_rayCst "4. Ray-Triangle Intersection"
    
    // Vertices
    const vec3& V0 = in_triangle->vertices[0];
    const vec3& V1 = in_triangle->vertices[1];
    const vec3& V2 = in_triangle->vertices[2];

    // Edge vectors
    const vec3& E1 = V1 - V0;
    const vec3& E2 = V2 - V0;

    // Face normal
    vec3 n = normalize(cross(E1, E2));

    // Ray origin and direction
    const vec3& P = in_ray.origin;
    const vec3& w = in_ray.direction;

    // Plane intersection what is q and a?
    vec3 q = cross(w, E2);
    float a = dot(E1, q);

    // Ray parallel or close to the limit of precision?
    if (fabsf(a) <= small_number) return false;

    // Detect backface
    // !!!! it works but should be the opposite! are faces left or right oriented?
    if ((dot(n, w) < 0))
    {
      out_hit.front_face = true;
    }
    else
    {
      out_hit.front_face = false;
      n = n * -1;
      if (drop_backface) return false;
    }

    // ?
    const vec3& s = (P - V0) / a;
    const vec3& r = cross(s, E1);

    // Barycentric coordinates
    float b[3];
    b[0] = dot(s, q);
    b[1] = dot(r, w);
    b[2] = 1.0f - (b[0] + b[1]);

    // Intersection outside of triangle?
    if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f)) return false;

    // Distance to intersection
    float t = dot(E2, r);

    // Intersection outside of ray range?
    if (t < t_min || t > t_max) return false;

    // Intersected inside triangle
    out_hit.t = t;
    out_hit.p = in_ray.at(t);

    // ?
    vec3 barycentric(b[2], b[0], b[1]);
    // this does not work, TODO translate normals?
    //out_hit.normal = normalize(barycentric[0] * in_triangle->normals[0] + barycentric[1] * in_triangle->normals[1] + barycentric[2] * in_triangle->normals[2]);
    out_hit.normal = n;

    const vec3& uv = barycentric[0] * in_triangle->UVs[0] + barycentric[1] * in_triangle->UVs[1] + barycentric[2] * in_triangle->UVs[2];
    out_hit.u = uv.x;
    out_hit.v = uv.y;

    return true;
  }
}

namespace random_seed
{
  float rand_iqint1(uint32_t seed)
  {
    static uint32_t last = 0;
    uint32_t state = seed + last;   // Seed can be the same for multiple calls, we need to rotate it
    state = (state << 13U) ^ state;
    state = state * (state * state * 15731U + 789221U) + 1376312589U;
    last = state;
    return (float)state / (float)UINT_MAX;   // [0.0f, 1.0f]
  }

  float rand_pcg(uint32_t seed)
  {
    static uint32_t last = 0;
    uint32_t state = seed + last;   // Seed can be the same for multiple calls, we need to rotate it
    state = state * 747796405U + 2891336453U;
    uint32_t word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
    uint32_t result = ((word >> 22U) ^ word);
    last = result;
    return (float)result / (float)UINT_MAX;   // [0.0f, 1.0f]
  }

  vec3 direction(uint32_t seed)
  {
    float x = normal_distribution(seed);
    float y = normal_distribution(seed);
    float z = normal_distribution(seed);
    return math::normalize(vec3(x, y, z));
  }
  
  float normal_distribution(uint32_t seed)
  {
    float theta = 2.0f * math::pi * RAND_SEED_FUNC(seed);
    float rho = sqrt(-2.0f * log(RAND_SEED_FUNC(seed)));
    assert(isfinite(rho));
    return rho * cos(theta);  // [-1.0f, 1.0f]
  }

  vec3 cosine_direction(uint32_t seed)
  {
    // https://psgraphics.blogspot.com/2013/11/random-directions-with-cosine.html
    // Cosine distribution around positive z axis
    float r1 = RAND_SEED_FUNC(seed);
    float r2 = RAND_SEED_FUNC(seed);
    //float phi = 2 * pi * r1;
    float r = 0.5;
    float x = cos(2 * math::pi * r1) * sqrt(r);
    float y = sin(2 * math::pi * r2) * sqrt(r);
    float z = sqrt(1 - r);
    return vec3(x, y, z);
  }
}

namespace random_cache
{
  template<typename T, int N>
  T cache<T, N>::get()
  {
    assert(last_index >= 0 && last_index < N);
    last_index = (last_index + 1) % N;
    return storage[last_index];
  }

  template<typename T, int N>
  void cache<T, N>::add(T value)
  {
    storage.push_back(value);
  }

  template<typename T, int N>
  int cache<T, N>::len()
  {
    return N;
  }

  void init()
  {
    // Fill float cache
    std::uniform_real_distribution<float> distribution;
    distribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    std::mt19937 generator(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
    for (int s = 0; s < float_cache.len(); s++)
    {
      float_cache.add(distribution(generator));
    }

    // Fill cosine direction cache
    for (int s = 0; s < cosine_direction_cache.len(); s++)
    {
      cosine_direction_cache.add(cosine_direction());
    } 
  }

  float get_float()
  {
    return float_cache.get();
  }

  float get_float_0_1()
  {
    return fabs(float_cache.get());
  }

  float get_float_0_N(float N)
  {
    return fabs(float_cache.get()) * N;
  }

  float get_float_M_N(float M, float N)
  {
    if (M < N)
    {
      return M + fabs(float_cache.get()) * (N - M);
    }
    return N + fabs(float_cache.get()) * (M - N);
  }

  vec3 get_vec3() // [-1,1]
  {
    return vec3(float_cache.get(), float_cache.get(), float_cache.get());
  }

  vec3 get_vec3_0_1()
  {
    return vec3(fabs(float_cache.get()), fabs(float_cache.get()), fabs(float_cache.get()));
  }

  int get_int_0_N(int N)
  {
    return (int)round(get_float_0_1() * N);
  }

  vec3 get_cosine_direction()
  {
    return cosine_direction_cache.get();
  }

  vec3 direction()
  {
    float x = normal_distribution();
    float y = normal_distribution();
    float z = normal_distribution();
    return math::normalize(vec3(x, y, z));
  }

  vec3 cosine_direction()
  {
    // https://psgraphics.blogspot.com/2013/11/random-directions-with-cosine.html
    // Cosine distribution around positive z axis
    float r1 = get_float_0_1();
    float r2 = get_float_0_1();
    //float phi = 2 * pi * r1;
    float r = 0.5;
    float x = cos(2 * math::pi * r1) * sqrt(r);
    float y = sin(2 * math::pi * r2) * sqrt(r);
    float z = sqrt(1 - r);
    return vec3(x, y, z);
  }

  vec3 in_sphere(float radius, float distance_squared)
  {
    float r1 = get_float_0_1();
    float r2 = get_float_0_1();
    float z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

    float phi = 2 * math::pi * r1;
    float x = cos(phi) * sqrt(1 - z * z);
    float y = sin(phi) * sqrt(1 - z * z);

    return vec3(x, y, z);
  }

  float normal_distribution()
  {
    float theta = 2.0f * math::pi * get_float_0_1();
    float rho = sqrt(-2.0f * log(get_float_0_1()));
    assert(isfinite(rho));
    return rho * cos(theta);  // [-1.0f, 1.0f]
  }
}

namespace tone_mapping
{
  vec3 trivial(const vec3& v)
  {
    return math::clamp_vec3(0.0f, 1.0f, v);
  }
  vec3 reinhard(const vec3& v)
  {
    // Mathematically guarantees to produce [0.0, 1.0]
    // Use with luminance not with RGB radiance
    return v / (1.0f + v);
  }
  vec3 reinhard_extended(const vec3& v, float max_white)
  {
    vec3 numerator = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
  }
  float luminance(const vec3& v)
  {
    float value = math::dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
    assert(value >= 0.0f);
    return value;
  }
  vec3 change_luminance(const vec3& c_in, float l_out)
  {
    float l_in = luminance(c_in);
    if (l_in == 0.0f)
    {
      return vec3(0, 0, 0);
    }
    return c_in * (l_out / l_in);
  }
  vec3 reinhard_extended_luminance(const vec3& v, float max_white_l)
  {
    assert(max_white_l > 0.0f);
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
  }
}

namespace hash
{
  uint32_t combine(uint32_t a, uint32_t b)
  {
    uint32_t c = 0x9e3779b9;
    a += c;

    a -= c; a -= b; a ^= (b >> 13);
    c -= b; c -= a; c ^= (a << 8);
    b -= a; b -= c; b ^= (c >> 13);
    a -= c; a -= b; a ^= (b >> 12);
    c -= b; c -= a; c ^= (a << 16);
    b -= a; b -= c; b ^= (c >> 5);
    a -= c; a -= b; a ^= (b >> 3);
    c -= b; c -= a; c ^= (a << 10);
    b -= a; b -= c; b ^= (c >> 15);

    return b;
  }
  uint32_t combine(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
  {
    return combine(combine(a, b), combine(c, d));
  }
  uint32_t get(uint64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }
  uint32_t get(int64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }
  uint32_t get(float a)
  {
    return reinterpret_cast<uint32_t&>(a);
  }
  uint32_t get(double a)
  {
    return get(reinterpret_cast<uint64_t&>(a));
  }
  uint32_t get(const void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }
  uint32_t get(void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }
  uint32_t get(bool a)
  {
    return (uint32_t)a;
  }
  uint32_t get(const vec3& a)
  {
    return combine(get(a.x), get(a.y), get(a.z), get(a.padding));
  }
  uint32_t get(const std::string& a)
  {
    return static_cast<uint32_t>(std::hash<std::string>{}(a));
  }
}

namespace io
{
  std::string get_working_dir()
  {
    std::string current_dir = std::filesystem::current_path().string();
    std::ostringstream oss;
    oss << current_dir << "\\";
    return oss.str();
  }

  std::string get_workspace_dir()
  {
    std::string working_dir = get_working_dir();
    std::ostringstream oss;
    oss << working_dir << "..\\..\\Workspace\\";
    return oss.str();
  }

  std::string get_content_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "Content\\";
    return oss.str();
  }

  std::string get_materials_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Materials\\";
    return oss.str();
  }

  std::string get_meshes_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Meshes\\";
    return oss.str();
  }

  std::string get_textures_dir()
  {
    std::string content_dir = get_content_dir();
    std::ostringstream oss;
    oss << content_dir << "Textures\\";
    return oss.str();
  }

  std::string get_images_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "Images\\";
    return oss.str();
  }


  std::string get_workspace_file_path(const char* file_name)
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << file_name;
    return oss.str();
  }

  std::string get_images_file_path(const char* file_name)
  {
    std::string images_dir = get_images_dir();
    std::ostringstream oss;
    oss << images_dir << file_name;
    return oss.str();
  }

  std::string get_material_file_path(const char* file_name)
  {
    std::string materials_dir = get_materials_dir();
    std::ostringstream oss;
    oss << materials_dir << file_name;
    return oss.str();
  }

  std::string get_mesh_file_path(const char* file_name)
  {
    std::string meshes_dir = get_meshes_dir();
    std::ostringstream oss;
    oss << meshes_dir << file_name;
    return oss.str();
  }

  std::string get_texture_file_path(const char* file_name)
  {
    std::string textures_dir = get_textures_dir();
    std::ostringstream oss;
    oss << textures_dir << file_name;
    return oss.str();
  }

  std::string get_window_file_path()
  {
    return get_workspace_file_path("window.json");
  }

  std::string get_scene_file_path()
  {
    return get_workspace_file_path("scene.json");
  }

  std::string get_rendering_file_path()
  {
    return get_workspace_file_path("rendering.json");
  }

  std::string get_imgui_file_path()
  {
    return get_workspace_file_path("imgui.ini");
  }

  std::string get_render_output_file_path()
  {
    std::time_t t = std::time(nullptr);
    std::ostringstream oss;
    oss << "output_" << t << ".bmp";
    return get_images_file_path(oss.str().c_str());
  }

  std::vector<std::string> discover_files(const std::string& path, const std::string& extension, bool include_extension)
  {
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_regular_file() && entry.path().extension() == extension)
      {
        std::string file_name = entry.path().filename().string();
        size_t index = file_name.find_last_of(".");
        result.push_back(file_name.substr(0, index));
      }
    }
    return result;
  }

  std::vector<std::string> discover_material_files(bool include_extension)
  {
    return discover_files(get_materials_dir(), ".json", include_extension);
  }
  
  std::vector<std::string> discover_texture_files(bool include_extension)
  {
    return discover_files(get_textures_dir(), ".json", include_extension);
  }

  std::vector<std::string> discover_mesh_files(bool include_extension)
  {
    return discover_files(get_meshes_dir(), ".json", include_extension);
  }
}

namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces)
  {
    assert(shape_index >= 0);

    tinyobj::attrib_t attributes; // not implemented
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; // not implemented

    std::string dir = io::get_meshes_dir();
    std::string path = io::get_mesh_file_path(file_name.c_str());

    std::string error;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.c_str(), dir.c_str(), true))
    {
      logger::error("Unable to load object file: {0}", path);
      return false;
    }
    if (shape_index >= shapes.size())
    {
      logger::error("Object file: {0} does not have shape index: {1}", path, shape_index);
      return false;
    }

    tinyobj::shape_t shape = shapes[shape_index];
    size_t num_faces = shape.mesh.num_face_vertices.size();
    if (num_faces == 0)
    {
      logger::error("Object file: {0} has no faces", path);
      return false;
    }
    out_faces.reserve(num_faces);

    // loop over faces
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
      out_faces.push_back(triangle_face());
      triangle_face& face = out_faces[fi];

      // loop over the vertices in the face
      assert(shape.mesh.num_face_vertices[fi] == 3);
      for (size_t vi = 0; vi < 3; ++vi)
      {
        tinyobj::index_t idx = shape.mesh.indices[3 * fi + vi];

        if (idx.vertex_index == -1)
        {
          logger::error("Object file: {0} faces not found", file_name);
          return false;
        }
        if (idx.normal_index == -1)
        {
          logger::error("Object file: {0} normals not found", file_name);
          return false;
        }
        if (idx.texcoord_index == -1)
        {
          logger::error("Object file: {0} UVs not found", file_name);
          return false;
        }

        float vx = attributes.vertices[3 * idx.vertex_index + 0];
        float vy = attributes.vertices[3 * idx.vertex_index + 1];
        float vz = attributes.vertices[3 * idx.vertex_index + 2];
        face.vertices[vi] = vec3(vx, vy, vz);

        float nx = attributes.normals[3 * idx.normal_index + 1];
        float ny = attributes.normals[3 * idx.normal_index + 2];
        float nz = attributes.normals[3 * idx.normal_index + 0];
        face.normals[vi] = vec3(nx, ny, nz);
        
        float uvx = attributes.texcoords[2 * idx.texcoord_index + 0];
        float uvy = attributes.texcoords[2 * idx.texcoord_index + 1];
        face.UVs[vi] = vec3(uvx, uvy, 0.0f);
      }
    }
    
    return true;
  }
}

namespace img_helper
{
  bool load_img(const std::string& file_name, int width, int height, texture* out_texture)
  {
    std::string path = io::get_texture_file_path(file_name.c_str());

    out_texture->is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));

    if (out_texture->is_hdr)
    {
      out_texture->data_hdr = stbi_loadf(path.c_str(), &width, &height, nullptr, STBI_rgb);
      if (out_texture->data_hdr == nullptr)
      {
        logger::error("Texture file: {0} failed to open", path);
        return false;
      }
    }
    else
    {
      out_texture->data_ldr = stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb);
      if (out_texture->data_ldr == nullptr)
      {
        logger::error("Texture file: {0} failed to open", path);
        return false;
      }
    }
    return true;
  }
}

namespace logger
{
  void init()
  {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
    //sinks.push_back(std::make_shared<spdlog::sinks::?>("logfile", 23, 59)); // TODO log to file
    auto combined_logger = std::make_shared<spdlog::logger>("console", begin(sinks), end(sinks));
    spdlog::register_logger(combined_logger);

    spdlog::flush_every(std::chrono::seconds(3));
#if BUILD_DEBUG
    spdlog::set_level(spdlog::level::trace);
#elif BUILD_RELEASE
    spdlog::set_level(spdlog::level::info);
#endif
    spdlog::set_pattern("[%H:%M:%S.%e] [thread %t] [%^%l%$] %v");

    // Test
    // logger::trace("trace");
    // logger::debug("debug");
    // logger::info("info");
    // logger::warn("warn");
    // logger::error("error");
    // logger::critical("critical");
  }
}