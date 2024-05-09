#include "ffbx.h"

#include <tchar.h>
#include <cstdio>
#include <fstream>
#include <sstream>

#include "engine/io.h"
#include "engine/log.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"
#include "nlohmann/json.hpp"

#include "nlohmann/json_fwd.hpp"
#include "persistence/object_persistence.h"

#include "third_party/ofbx.h"

namespace engine
{
    bool ffbx::save_as_obj(const ofbx::Mesh& mesh, const char* path)
	{
		FILE* fp = fopen(path, "wb");
		if (!fp) return false;
		int indices_offset = 0;
		int mesh_idx = 0;
    	
		const ofbx::GeometryData& geom = mesh.getGeometryData();
		const ofbx::Vec3Attributes positions = geom.getPositions();
		const ofbx::Vec3Attributes normals = geom.getNormals();
		const ofbx::Vec2Attributes uvs = geom.getUVs();
		
		// each ofbx::Mesh can have several materials == partitions
		for (int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx) {
			fprintf(fp, "o obj%d_%d\ng grp%d\n", mesh_idx, partition_idx, mesh_idx);
			const ofbx::GeometryPartition& partition = geom.getPartition(partition_idx);
		
			// partitions most likely have several polygons, they are not triangles necessarily, use ofbx::triangulate if you want triangles
			for (int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
				const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];
				
				for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
					ofbx::Vec3 v = positions.get(i);
					fprintf(fp, "v %f %f %f\n", v.x, v.y, v.z);
				}
		
				bool has_normals = normals.values != nullptr;
				if (has_normals) {
					// normals.indices might be different than positions.indices
					// but normals.get(i) is normal for positions.get(i)
					for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
						ofbx::Vec3 n = normals.get(i);
						fprintf(fp, "vn %f %f %f\n", n.x, n.y, n.z);
					}
				}
		
				bool has_uvs = uvs.values != nullptr;
				if (has_uvs) {
					for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
						ofbx::Vec2 uv = uvs.get(i);
						fprintf(fp, "vt %f %f\n", uv.x, uv.y);
					}
				}
			}
		
			for (int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
				const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];
				fputs("f ", fp);
				for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
					fprintf(fp, "%d ", 1 + i + indices_offset);
				}
				fputs("\n", fp);
			}
		
			indices_offset += positions.count;
		}
		
		fclose(fp);
		return true;
	}

	void ffbx::import_material_from_blender_4_1_principled_brdf(const ofbx::Material* material, fmaterial_properties& out_material_properties)
    {
    	// Base Color on the Principled BRDF node is saved in FBX as Specular
    	// Use it as Specular and Diffuse here
    	
    	//ofbx::Color emissive = material->getEmissiveColor();
    	//ofbx::Color diffuse = material->getDiffuseColor();	
    	//ofbx::Color ambient = material->getAmbientColor();
    	ofbx::Color specular = material->getSpecularColor();

    	//out_material_properties.emissive = { emissive.r, emissive.g, emissive.b, 1.0f };
    	out_material_properties.diffuse = { specular.r, specular.g, specular.b, 1.0f };
    	//out_material_properties.ambient = { ambient.r, ambient.g, ambient.b, 1.0f };
    	out_material_properties.specular = { specular.r, specular.g, specular.b, 1.0f };
    }
	
    void ffbx::load_fbx(hscene* scene_object)
    {
        std::string content_dir = fio::get_content_dir();
    	std::string meshes_dir = fio::get_meshes_dir();
        std::ostringstream fbx_file;
        fbx_file << content_dir << "fbx_scene_test.fbx";
        
        FILE* fp = fopen(fbx_file.str().c_str(), "rb");
        if (!fp) return;

        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto* content = new ofbx::u8[file_size];
        fread(content, 1, file_size, fp);
        
        // Ignoring certain nodes will only stop them from being processed not tokenised (i.e. they will still be in the tree)
        ofbx::LoadFlags flags =
        	//ofbx::LoadFlags::IGNORE_MODELS | ofbx::LoadFlags::IGNORE_TEXTURES | ofbx::LoadFlags::IGNORE_MATERIALS | ofbx::LoadFlags::IGNORE_MESHES |
            ofbx::LoadFlags::IGNORE_SKIN | ofbx::LoadFlags::IGNORE_BONES | ofbx::LoadFlags::IGNORE_PIVOTS |
            ofbx::LoadFlags::IGNORE_BLEND_SHAPES | ofbx::LoadFlags::IGNORE_CAMERAS | ofbx::LoadFlags::IGNORE_LIGHTS |
            ofbx::LoadFlags::IGNORE_POSES | ofbx::LoadFlags::IGNORE_VIDEOS | ofbx::LoadFlags::IGNORE_LIMBS |
            ofbx::LoadFlags::IGNORE_ANIMATIONS;

    	LOG_ERROR("Opening fbx: {0}", fbx_file.str());
    	
        ofbx::IScene*  g_scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u16)flags);

    	// a registry of already created asset resources
    	std::vector<std::string> g_material_assets;
    	
        for(int i = 0; i < g_scene->getMeshCount(); i++)
        {
        	// Iterate over scene objects, not geometry assets
        	// FBX constains one geometry object per mesh, even if they are the same meshes
            const ofbx::Mesh* mesh = g_scene->getMesh(i);
        	        	
        	// Get static mesh, export object file and save json
        	// Mesh assset object will be available in the object registry at the end
	        {
            	astatic_mesh* mesh_object = astatic_mesh::spawn();
            	mesh_object->file_name = mesh->name;

            	// Export obj file
            	std::ostringstream mesh_obj_file;
	            {
            		std::ostringstream mesh_obj_file_path;
            		mesh_obj_file_path << meshes_dir << mesh->name << ".obj";
            		mesh_obj_file << mesh->name << ".obj";
            		save_as_obj(*mesh, mesh_obj_file_path.str().c_str());
	            }
            	mesh_object->obj_file_name = mesh_obj_file.str();
            	
            	astatic_mesh::save(mesh_object);
            	astatic_mesh::load(mesh_object, mesh->name);
	        }

        	// Get all materials in a mesh and save json
        	// Material object will be available in the object registry at the end
            {
            	for(int y = 0; y < mesh->getMaterialCount(); y++)
            	{
					const ofbx::Material* material = mesh->getMaterial(y);
            		if(std::find(g_material_assets.begin(), g_material_assets.end(), material->name) != g_material_assets.end())
            		{
            			continue;
            		}
            		g_material_assets.push_back(material->name);
            		
            		amaterial* material_object = amaterial::spawn();
            		material_object->file_name = material->name;

            		import_material_from_blender_4_1_principled_brdf(material, material_object->properties);
            		
            		material_object->properties.use_texture = false;
            		
            		amaterial::save(material_object);
            		amaterial::load(material_object, material->name);
            	}
            }

        	// Scene object - spawn it
            {
            	float scale = 0.01f;
            	float flip_z = -1;
            	hstatic_mesh* object = hstatic_mesh::spawn();
            	std::ostringstream display_name;
            	display_name << mesh->name << i;
            	object->set_display_name(display_name.str());
            	object->mesh_asset_ptr.set_name(mesh->name);
            	object->material_asset_ptr.set_name(mesh->getMaterial(0)->name);
            	object->origin = fvec3(mesh->getLocalTranslation().x * scale, mesh->getLocalTranslation().y * scale, flip_z * mesh->getLocalTranslation().z* scale);
            	object->rotation = fvec3(mesh->getLocalRotation().x, mesh->getLocalRotation().y, mesh->getLocalRotation().z);
            	object->scale = fvec3(mesh->getLocalScaling().x* scale, mesh->getLocalScaling().y* scale, mesh->getLocalScaling().z* scale);
            	object->load_resources();
            	
            	scene_object->add(object);
            }
        	
        	//scene_object->load_resources();
        }
        delete[] content;
        fclose(fp);
    }

}