#include "stdafx.h"

#include "ffbx.h"

#include <tchar.h>
#include <cstdio>
#include <fstream>
#include <sstream>

#include "engine/io.h"
#include "engine/log.h"
#include "engine/string_tools.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "nlohmann/json.hpp"
#include "assets/mesh.h"
#include "assets/material.h"

#include "assimp/Importer.hpp"
#include "assimp/Exporter.hpp"
#include "assimp/LogStream.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

/*
 * Summary after 3 evenings of struggle with FBX importing - Hacky code here!
 * 
 * I checked Blender 4.1, Unity 2022.3.27f1 and Unreal Engine 5.3
 *
 * CONCLUSION:
 *  For Unreal and Unity:
 *  Assimp seems to be good for now. To improve, we need to merge both:
 *  Recognize the hierarchical structure with ofbx but save object files with assimp.
 *  This will be good start to begin with autoloding and auto instancing.
 *  For now meshes are scene wide merged blobs.
 *  For Blender:
 *  Assimp is good.
 *  
 * A BIT MORE DETAIL:
 *  - ofbx
 *  -- recognizes each primitive as a separate mesh
 *  -- I think it fails to save obj files properly as assimp loader screams with "T8620: OBJ: vertex index out of range" error and fails to load part of generated obj files.
 *  - assimp
 *  -- handles saving obj better, so the scene is complete
 *  -- fails to read hierarchical structure
 *  -- all meshes of the same type are merged into one geometry
 *
 * A LOT OF DETAIL:
 *  1. Blender (blender-3.5-splash.fbx), exports flat structure
 *  - ofbx
 *  -- simple material data like base color can be imported despire using Principled BRDF node
 *  -- scene is half broken
 *  - assimp
 *  -- recognized each primitive as a separate mesh
 *  -- saved obj files properly so the scene is complete, relatively low amount of warnings
 *  -- workable scene for future
 *    
 *  2. Unity (unity_terminal_scene.fbx), exports hierarchical structure
 *  - ofbx
 *  -- scene is half broken
 *  - assimp
 *  -- batches primitives together, but also has child elements, I need to investigate them
 *  -- "T24456: FBX-DOM: unsupported, newer format version, supported are only FBX 2011, FBX 2012 and FBX 2013, trying to read it nevertheless"
 *  -- mesh names are not preserved scene0, scene1 etc.
 *  -- workable scene for future
 *    
 *  3. Unreal (SunTemple.fbx), exports ? structure
 *  - ofbx
 *  -- scene is rotated so floors are walls
 *  -- some weird UV issues
 *  -- scene not workable
 *  - assimp
 *  -- unreal uses FBX 2013 format!
 *  -- all meshes of the same type are merged into one geometry
 *  -- workable scene for future
 */

namespace engine
{
  const unsigned int assimp_import_flags =
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_SortByPType |
    aiProcess_PreTransformVertices |
    aiProcess_GenNormals |
    aiProcess_GenUVCoords |
    aiProcess_OptimizeMeshes |
    aiProcess_Debone |
    aiProcess_GenBoundingBoxes;

  //void ffbx::save_as_obj_ofbx(const ofbx::Mesh& mesh, const char* path)
  //{
  //  FILE* fp = fopen(path, "wb");
  //  if(!fp) return;
  //  int indices_offset = 0;
  //  int mesh_idx = 0;
  //
  //  const ofbx::GeometryData& geom = mesh.getGeometryData();
  //  const ofbx::Vec3Attributes positions = geom.getPositions();
  //  const ofbx::Vec3Attributes normals = geom.getNormals();
  //  const ofbx::Vec2Attributes uvs = geom.getUVs();
  //
  //  // each ofbx::Mesh can have several materials == partitions
  //  for(int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx)
  //  {
  //    fprintf(fp, "o obj%d_%d\ng grp%d\n", mesh_idx, partition_idx, mesh_idx);
  //    const ofbx::GeometryPartition& partition = geom.getPartition(partition_idx);
  //
  //    // partitions most likely have several polygons, they are not triangles necessarily, use ofbx::triangulate if you want triangles
  //    for(int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx)
  //    {
  //      const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];
  //
  //      for(int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
  //      {
  //        ofbx::Vec3 v = positions.get(i);
  //        fprintf(fp, "v %f %f %f\n", v.x, v.y, v.z);
  //      }
  //
  //      bool has_normals = normals.values != nullptr;
  //      if(has_normals)
  //      {
  //        // normals.indices might be different than positions.indices
  //        // but normals.get(i) is normal for positions.get(i)
  //        for(int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
  //        {
  //          ofbx::Vec3 n = normals.get(i);
  //          fprintf(fp, "vn %f %f %f\n", n.x, n.y, n.z);
  //        }
  //      }
  //
  //      bool has_uvs = uvs.values != nullptr;
  //      if(has_uvs)
  //      {
  //        for(int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
  //        {
  //          ofbx::Vec2 uv = uvs.get(i);
  //          fprintf(fp, "vt %f %f\n", uv.x, uv.y);
  //        }
  //      }
  //    }
  //
  //    for(int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx)
  //    {
  //      const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];
  //      fputs("f ", fp);
  //      for(int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i)
  //      {
  //        fprintf(fp, "%d ", 1 + i + indices_offset);
  //      }
  //      fputs("\n", fp);
  //    }
  //
  //    indices_offset += positions.count;
  //  }
  //
  //  fclose(fp);
  //}

  void ffbx::save_as_obj_assimp(aiMesh* mesh, aiMaterial* material, const char* path)
  {
    // Build fake scene, attach material to it and save as OBJ
    aiScene* obj_scene = new aiScene();
    obj_scene->mRootNode = new aiNode();

    obj_scene->mMeshes = new aiMesh*[1];
    obj_scene->mMeshes[0] = nullptr;
    obj_scene->mNumMeshes = 1;
    obj_scene->mMeshes[0] = mesh;

    obj_scene->mMaterials = new aiMaterial*[1];
    obj_scene->mMaterials[0] = nullptr;
    obj_scene->mNumMaterials = 1;
    obj_scene->mMaterials[0] = material;

    obj_scene->mRootNode->mMeshes = new unsigned int[1];
    obj_scene->mRootNode->mMeshes[0] = 0;
    obj_scene->mRootNode->mNumMeshes = 1;

    obj_scene->mMeshes[0]->mMaterialIndex = 0;

    Assimp::Exporter exporter;

    if(exporter.Export(obj_scene, "obj", path) == aiReturn_FAILURE)
    {
      LOG_ERROR("Failed to export OBJ {0}", path);
    }
  }

  //void ffbx::import_material_from_blender_4_1_principled_brdf(const ofbx::Material* material, fmaterial_properties& out_material_properties)
  //{
  //  // Base Color on the Principled BRDF node is saved in FBX as Specular
  //  // Use it as Specular and Diffuse here
  //
  //  ofbx::Color emissive = material->getEmissiveColor();
  //  ofbx::Color diffuse = material->getDiffuseColor();
  //  ofbx::Color ambient = material->getAmbientColor();
  //  ofbx::Color specular = material->getSpecularColor();
  //
  //  //out_material_properties.emissive = { emissive.r, emissive.g, emissive.b, 1.0f };
  //  out_material_properties.diffuse = {specular.r, specular.g, specular.b, 1.0f};
  //  //out_material_properties.ambient = { ambient.r, ambient.g, ambient.b, 1.0f };
  //  out_material_properties.specular = {specular.r, specular.g, specular.b, 1.0f};
  //}

  //void ffbx::import_material_from_unity_2022_3_urp(const ofbx::Material* material, fmaterial_properties& out_material_properties)
  //{
  //  // Base Color on the URP is saved in FBX as Diffuse
  //  // Use it as Specular and Diffuse here
  //
  //  ofbx::Color emissive = material->getEmissiveColor();
  //  ofbx::Color diffuse = material->getDiffuseColor();
  //  ofbx::Color ambient = material->getAmbientColor();
  //  ofbx::Color specular = material->getSpecularColor();
  //
  //  //out_material_properties.emissive = { emissive.r, emissive.g, emissive.b, 1.0f };
  //  out_material_properties.diffuse = {diffuse.r, diffuse.g, diffuse.b, 1.0f};
  //  out_material_properties.ambient = {ambient.r, ambient.g, ambient.b, 1.0f};
  //  out_material_properties.specular = {diffuse.r, diffuse.g, diffuse.b, 1.0f};
  //}

  void print_hierarhy(aiNode* node, int level)
  {
    LOG_INFO("level:{0} children:{1}", level, node->mNumChildren);
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode* c = node->mChildren[i];
      LOG_INFO("level:{0} meshes:{1}", c->mName.C_Str(), c->mNumMeshes);
      print_hierarhy(c, level + 1);
    }
  }

  void ffbx::load_fbx_assimp(const std::string& file_name, hscene* scene_object)
  {
    std::string project_dir = fio::get_project_dir();
    std::string meshes_dir = fio::get_meshes_dir();
    std::ostringstream fbx_file;
    fbx_file << project_dir << file_name;

    Assimp::Importer importer;

    const aiScene* ai_scene = importer.ReadFile(fbx_file.str(), assimp_import_flags);
    if(!ai_scene)
    {
      LOG_ERROR("Unable to find file {0} in the content directory", file_name);
      return;
    }

    // a registry of already created asset resources
    std::vector<std::string> g_material_assets;

    print_hierarhy(ai_scene->mRootNode, 0);

    for(uint32_t i = 0; i < ai_scene->mNumMeshes; i++)
    {
      // Iterate over scene objects, not geometry assets
      // FBX constains one geometry object per mesh, even if they are the same meshes
      aiMesh* mesh = ai_scene->mMeshes[i];
      std::string mesh_name = mesh->mName.C_Str();
      fstring_tools::replace(mesh_name, "::", "_");
      fstring_tools::replace(mesh_name, "|", "_");
      std::ostringstream temp;
      temp << mesh_name << i;
      mesh_name = temp.str();

      // Get static mesh, export object file and save json
      // Mesh assset object will be available in the object registry at the end
      {
        astatic_mesh* mesh_object = astatic_mesh::spawn();
        mesh_object->name = mesh_name;

        // Export obj file
        std::ostringstream mesh_obj_file;
        {
          std::ostringstream mesh_obj_file_path;
          mesh_obj_file_path << meshes_dir << mesh_name << ".obj";
          mesh_obj_file << mesh_name << ".obj";

          save_as_obj_assimp(mesh, ai_scene->mMaterials[mesh->mMaterialIndex], mesh_obj_file_path.str().c_str());
        }
        mesh_object->obj_file_name = mesh_obj_file.str();

        mesh_object->save();
        mesh_object->load(mesh_name);
      }

      // Scene object - spawn it
      {
        float scale = 0.01f;
        float flip_z = -1;
        hstatic_mesh* object = hstatic_mesh::spawn();
        std::ostringstream display_name;
        display_name << mesh_name << i;
        object->set_display_name(display_name.str());
        object->mesh_asset_ptr.set_name(mesh_name);
        object->material_asset_ptr.set_name("default");
        object->origin = fvec3(0.0f, 0.0f, 0.0f);
        object->rotation = fvec3(0.0f, 0.0f, 0.0f);
        object->scale = fvec3(scale, scale, scale);
        object->load_resources();

        scene_object->add(object);
      }
    }
  }

  //void ffbx::load_fbx_ofbx(const std::string& file_name, hscene* scene_object)
  //{
  //  std::string project_dir = fio::get_project_dir();
  //  std::string meshes_dir = fio::get_meshes_dir();
  //  std::ostringstream fbx_file;
  //  fbx_file << project_dir << file_name;
  //
  //  FILE* fp = fopen(fbx_file.str().c_str(), "rb");
  //  if(!fp)
  //  {
  //    LOG_ERROR("Unable to find file {0} in the content directory", file_name);
  //    return;
  //  }
  //
  //  fseek(fp, 0, SEEK_END);
  //  long file_size = ftell(fp);
  //  fseek(fp, 0, SEEK_SET);
  //  auto* content = new ofbx::u8[file_size];
  //  fread(content, 1, file_size, fp);
  //
  //  // Ignoring certain nodes will only stop them from being processed not tokenised (i.e. they will still be in the tree)
  //  ofbx::LoadFlags flags =
  //    //ofbx::LoadFlags::IGNORE_MODELS | ofbx::LoadFlags::IGNORE_TEXTURES | ofbx::LoadFlags::IGNORE_MATERIALS | ofbx::LoadFlags::IGNORE_MESHES |
  //    ofbx::LoadFlags::IGNORE_SKIN | ofbx::LoadFlags::IGNORE_BONES | ofbx::LoadFlags::IGNORE_PIVOTS |
  //    ofbx::LoadFlags::IGNORE_BLEND_SHAPES | ofbx::LoadFlags::IGNORE_CAMERAS | ofbx::LoadFlags::IGNORE_LIGHTS |
  //    ofbx::LoadFlags::IGNORE_POSES | ofbx::LoadFlags::IGNORE_VIDEOS | ofbx::LoadFlags::IGNORE_LIMBS |
  //    ofbx::LoadFlags::IGNORE_ANIMATIONS;
  //
  //  LOG_ERROR("Opening fbx: {0}", fbx_file.str());
  //
  //  ofbx::IScene* g_scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u16)flags);
  //
  //  // a registry of already created asset resources
  //  std::vector<std::string> g_material_assets;
  //
  //  for(int i = 0; i < g_scene->getMeshCount(); i++)
  //  {
  //    // Iterate over scene objects, not geometry assets
  //    // FBX constains one geometry object per mesh, even if they are the same meshes
  //    const ofbx::Mesh* mesh = g_scene->getMesh(i);
  //    std::string mesh_name = mesh->name;
  //    fstring_tools::replace(mesh_name, "::", "_");
  //    fstring_tools::replace(mesh_name, "|", "_");
  //
  //    // Get static mesh, export object file and save json
  //    // Mesh assset object will be available in the object registry at the end
  //    {
  //      astatic_mesh* mesh_object = astatic_mesh::spawn();
  //      mesh_object->name = mesh_name;
  //
  //      // Export obj file
  //      std::ostringstream mesh_obj_file;
  //      {
  //        std::ostringstream mesh_obj_file_path;
  //        mesh_obj_file_path << meshes_dir << mesh_name << ".obj";
  //        mesh_obj_file << mesh_name << ".obj";
  //        save_as_obj_ofbx(*mesh, mesh_obj_file_path.str().c_str());
  //      }
  //      mesh_object->obj_file_name = mesh_obj_file.str();
  //
  //      mesh_object->save();
  //      mesh_object->load(mesh_name);
  //    }
  //
  //    // Get all materials in a mesh and save json
  //    // Material object will be available in the object registry at the end
  //    {
  //      for(int y = 0; y < mesh->getMaterialCount(); y++)
  //      {
  //        const ofbx::Material* material = mesh->getMaterial(y);
  //        std::string material_name = material->name;
  //        fstring_tools::replace(mesh_name, "::", "_");
  //        fstring_tools::replace(mesh_name, "|", "_");
  //
  //        if(std::find(g_material_assets.begin(), g_material_assets.end(), material_name) != g_material_assets.end())
  //        {
  //          continue;
  //        }
  //        g_material_assets.push_back(material_name);
  //
  //        amaterial* material_object = amaterial::spawn();
  //        material_object->name = "default"; //material_name;
  //
  //        import_material_from_unity_2022_3_urp(material, material_object->properties);
  //
  //        material_object->save();
  //        material_object->load(material_name);
  //      }
  //    }
  //
  //    // Scene object - spawn it
  //    {
  //      double scale = 0.01;
  //      double flip_z = -1;
  //      hstatic_mesh* object = hstatic_mesh::spawn();
  //      std::ostringstream display_name;
  //      display_name << mesh_name << i;
  //      object->set_display_name(display_name.str());
  //      object->mesh_asset_ptr.set_name(mesh_name);
  //      if(mesh->getMaterialCount() > 0)
  //      {
  //        //std::string material_name = mesh->getMaterial(0)->name;
  //        //fstring_tools::replace(mesh_name, "::", "_");
  //        //fstring_tools::replace(mesh_name, "|", "_");
  //        //object->material_asset_ptr.set_name(material_name);
  //        object->material_asset_ptr.set_name("default");
  //      }
  //      object->origin = fvec3(mesh->getLocalTranslation().x * scale, mesh->getLocalTranslation().y * scale, flip_z * mesh->getLocalTranslation().z * scale);
  //      object->rotation = fvec3(mesh->getLocalRotation().x, mesh->getLocalRotation().y, mesh->getLocalRotation().z);
  //      object->scale = fvec3(mesh->getLocalScaling().x * scale, mesh->getLocalScaling().y * scale, mesh->getLocalScaling().z * scale);
  //      object->load_resources();
  //
  //      scene_object->add(object);
  //    }
  //
  //    //scene_object->load_resources();
  //  }
  //  delete[] content;
  //  fclose(fp);
  //}
}
