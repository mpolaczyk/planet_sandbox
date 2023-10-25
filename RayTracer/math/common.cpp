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





namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces)
  {
    assert(shape_index >= 0);

    tinyobj::attrib_t attributes; // not implemented
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; // not implemented

    std::string dir = engine::io::get_meshes_dir();
    std::string path = engine::io::get_mesh_file_path(file_name.c_str());

    std::string error;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.c_str(), dir.c_str(), true))
    {
      LOG_ERROR("Unable to load object file: {0} {1}", path, error);
      return false;
    }
    if (shape_index >= shapes.size())
    {
      LOG_ERROR("Object file: {0} does not have shape index: {1}", path, shape_index);
      return false;
    }

    tinyobj::shape_t shape = shapes[shape_index];
    size_t num_faces = shape.mesh.num_face_vertices.size();
    if (num_faces == 0)
    {
      LOG_ERROR("Object file: {0} has no faces", path);
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
          LOG_ERROR("Object file: {0} faces not found", file_name);
          return false;
        }
        if (idx.normal_index == -1)
        {
          LOG_ERROR("Object file: {0} normals not found", file_name);
          return false;
        }
        if (idx.texcoord_index == -1)
        {
          LOG_ERROR("Object file: {0} UVs not found", file_name);
          return false;
        }

        float vx = attributes.vertices[3 * idx.vertex_index + 0];
        float vy = attributes.vertices[3 * idx.vertex_index + 1];
        float vz = attributes.vertices[3 * idx.vertex_index + 2];
        face.vertices[vi] = vec3(vx, vy, vz);

        float nx = attributes.normals[3 * idx.normal_index + 1];
        float ny = attributes.normals[3 * idx.normal_index + 2];
        float nz = attributes.normals[3 * idx.normal_index + 0];
        face.normals[vi] = math::normalize(vec3(nx, ny, nz));
        
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
    std::string path = engine::io::get_texture_file_path(file_name.c_str());

    out_texture->is_hdr = static_cast<bool>(stbi_is_hdr(path.c_str()));

    if (out_texture->is_hdr)
    {
      out_texture->data_hdr = stbi_loadf(path.c_str(), &width, &height, nullptr, STBI_rgb);
      if (out_texture->data_hdr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
    }
    else
    {
      out_texture->data_ldr = stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb);
      if (out_texture->data_ldr == nullptr)
      {
        LOG_ERROR("Texture file: {0} failed to open", path);
        return false;
      }
    }
    return true;
  }
}

