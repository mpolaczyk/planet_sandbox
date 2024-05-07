#include <d3d11_1.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

#include "engine/log.h"
#include "hittables/light.h"
#include "hittables/scene.h"
#include "hittables/static_mesh.h"

using namespace DirectX;

namespace engine
{
    OBJECT_DEFINE(rgpu, rrenderer_base, GPU renderer)
    OBJECT_DEFINE_SPAWN(rgpu)
    
    void rgpu::init()
    {
        // FIX temporary hack here! It should be properly persistent as part for the scene
        vertex_shader_asset.set_name("gpu_renderer_vs");
        if(!vertex_shader_asset.get()) 
        {
            throw std::runtime_error("Failed to load vertex shader");
        }

        pixel_shader_asset.set_name("gpu_renderer_ps");
        if(!pixel_shader_asset.get())
        {
            throw std::runtime_error("Failed to load pixel shader");
        }

        default_material.set_name("default");
        if(!default_material.get())
        {
            throw std::runtime_error("Failed to load default material");
        }

        auto dx = fdx11::instance();
        {
            D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
            {
                // Per-vertex
                {"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
            auto blob = vertex_shader_asset.get()->render_state.shader_blob;
            dx.create_input_layout(input_element_desc, ARRAYSIZE(input_element_desc), blob, input_layout);
        }
        dx.create_sampler_state(sampler_state);
        dx.create_constant_buffer(sizeof(fobject_data), object_constant_buffer);
        dx.create_constant_buffer(sizeof(fframe_data), frame_constant_buffer);
        dx.create_rasterizer_state(rasterizer_state);
        dx.create_depth_stencil_state(depth_stencil_state);
    }

    void rgpu::render_frame_impl()
    {
        fdx11& dx = fdx11::instance();
        
        dx.device_context->ClearRenderTargetView(output_rtv.Get(), scene->clear_color);
        dx.device_context->ClearDepthStencilView(output_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        // Poke static mesh loading and build scene caches
        {
            material_map.clear();
            material_map.reserve(MAX_MATERIALS);
            materials.clear();
            materials.reserve(MAX_MATERIALS);
            next_material_id = 0;
            meshes.clear();
            lights.clear();
            lights.reserve(MAX_LIGHTS);
            next_light_id = 0;
            
            for (const hhittable_base* hittable : scene->objects)
            {
                if(hittable->get_class() == hstatic_mesh::get_class_static())
                {
                    const hstatic_mesh* mesh = static_cast<const hstatic_mesh*>(hittable);
                    meshes.push_back(mesh);
                    volatile const astatic_mesh* mesh_asset = mesh->mesh_asset_ptr.get();
                    const amaterial* material = mesh->material_asset_ptr.get();
                    if(material && !material_map.contains(material))
                    {
                        material_map.insert(std::pair<const amaterial*, uint32_t>(material, next_material_id));
                        materials.push_back(material);
                        next_material_id++;
                        if(next_material_id > MAX_MATERIALS)
                        {
                            LOG_ERROR("Unable to render more materials than MAX_MATERIALS");
                            return;
                        }
                    }    
                }
                else if(hittable->get_class() == hlight::get_class_static())
                {
                    const hlight* light = static_cast<const hlight*>(hittable);
                    if(light->properties.enabled)
                    {
                        lights.push_back(light);
                        next_light_id++;
                        if(next_light_id > MAX_LIGHTS)
                        {
                            LOG_ERROR("Unable to render more lights than MAX_LIGHTS");
                            return;
                        }
                    }
                }
            }
            if(lights.size() == 0)
            {
                LOG_ERROR("Scene is missing light");
                return;
            }
        }

        const D3D11_VIEWPORT viewport = {0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f};
        dx.device_context->RSSetViewports(1, &viewport);
        dx.device_context->RSSetState(rasterizer_state.Get());
        dx.device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);
        dx.device_context->OMSetRenderTargets(1, output_rtv.GetAddressOf(), output_dsv.Get());

        dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        dx.device_context->IASetInputLayout(input_layout.Get());
        
        dx.device_context->VSSetShader(vertex_shader_asset.get()->render_state.shader.Get(), nullptr, 0);
        dx.device_context->VSSetConstantBuffers(0, 1, object_constant_buffer.GetAddressOf());
        
        dx.device_context->PSSetShader(pixel_shader_asset.get()->render_state.shader.Get(), nullptr, 0);
        dx.device_context->PSSetConstantBuffers(0, 1, frame_constant_buffer.GetAddressOf());
        dx.device_context->PSSetConstantBuffers(1, 1, object_constant_buffer.GetAddressOf());
        
        dx.device_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());
        
        // Update per-frame constant buffer
        {
            fframe_data pfd;
            pfd.camera_position = XMFLOAT4(camera.location.e);
            pfd.ambient_light =  scene->ambient_light_color;
            for(uint32_t i = 0; i < MAX_LIGHTS; i++)
            {
                if(i < next_light_id)
                {
                    pfd.lights[i] = lights[i]->properties;
                    pfd.lights[i].position = XMFLOAT4(lights[i]->origin.e);
                }
                else
                {
                    pfd.lights[i] = flight_properties();
                }
            }
            for(uint32_t i = 0; i < MAX_MATERIALS; i++)
            {
                pfd.materials[i] = i < next_material_id ? materials[i]->properties : fmaterial_properties();
            }
            dx.update_constant_buffer<fframe_data>(&pfd, frame_constant_buffer);
        }
        
        // Draw the scene
        for (const hstatic_mesh* sm : meshes)
        {
            const astatic_mesh* sma = sm->mesh_asset_ptr.get();
            if (sma == nullptr) { continue; }
            const fstatic_mesh_render_state& smrs = sma->render_state;
            const amaterial* ma = sm->material_asset_ptr.get();
            if (ma == nullptr)
            {
                ma = default_material.get();
            }
            const atexture* ta = ma->texture_asset_ptr.get();
            
            // Update per-object constant buffer
            {
                XMMATRIX translation_matrix = XMMatrixTranslation(sm->origin.x, sm->origin.y, sm->origin.z );
                XMMATRIX rotation_matrix = XMMatrixRotationX(XMConvertToRadians(sm->rotation.x))
                    * XMMatrixRotationY(XMConvertToRadians(sm->rotation.y))
                    * XMMatrixRotationZ(XMConvertToRadians(sm->rotation.z));
                XMMATRIX scale_matrix = XMMatrixScaling(sm->scale.x, sm->scale.y, sm->scale.z);
                XMMATRIX world_matrix = scale_matrix * rotation_matrix * translation_matrix;
                
                const XMMATRIX inverse_transpose_model_world = XMMatrixTranspose(XMMatrixInverse(nullptr, world_matrix));
                const XMMATRIX model_world_view_projection = XMMatrixMultiply(world_matrix, XMLoadFloat4x4(&camera.view_projection));
            
                fobject_data pod;
                XMStoreFloat4x4(&pod.model_world, world_matrix);
                XMStoreFloat4x4(&pod.inverse_transpose_model_world, inverse_transpose_model_world);
                XMStoreFloat4x4(&pod.model_world_view_projection, model_world_view_projection);
                if(const amaterial* material = sm->material_asset_ptr.get())
                {
                    pod.material_id = material_map[material];
                }
                dx.update_constant_buffer<fobject_data>(&pod, object_constant_buffer);
            }

            // Update texture
            if(ma->properties.use_texture)
            {
                if(ta == nullptr)
                {
                    ta = default_material.get()->texture_asset_ptr.get();
                }
                dx.device_context->PSSetShaderResources(0, 1, ta->render_state.texture_srv.GetAddressOf());
            }
            
            // Update mesh
            dx.device_context->IASetVertexBuffers(0, 1, smrs.vertex_buffer.GetAddressOf(), &smrs.stride, &smrs.offset);
            dx.device_context->IASetIndexBuffer(smrs.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            static_assert(sizeof(fface_data_type) == sizeof(uint32_t));

            // Draw
            dx.device_context->DrawIndexed(smrs.num_indices, 0, 0);
        }
    }
}
