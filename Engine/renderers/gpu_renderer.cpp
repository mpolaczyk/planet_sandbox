#include <d3d11_1.h>
#include <DirectXColors.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

#include "engine/log.h"
#include "hittables/static_mesh.h"

using namespace DirectX;

namespace engine
{
    OBJECT_DEFINE(rgpu, rrenderer_base, GPU renderer)
    OBJECT_DEFINE_SPAWN(rgpu)

    void rgpu::create_output_texture(bool cleanup)
    {
        if (cleanup)
        {
            DX_RELEASE(output_rtv)
            DX_RELEASE(output_srv)
            DX_RELEASE(output_dsv)
            DX_RELEASE(output_texture)
            DX_RELEASE(output_depth_texture)
        }
        
        fdx11& dx = fdx11::instance();
        
        // Output and depth texture
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = output_width;
            desc.Height = output_height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            if(FAILED(dx.device->CreateTexture2D(&desc, NULL, &output_texture)))
            {
                throw std::runtime_error("CreateTexture2D output texture failed.");
            }
            desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if(FAILED(dx.device->CreateTexture2D(&desc, NULL, &output_depth_texture)))
            {
                throw std::runtime_error("CreateTexture2D output depth texture failed.");
            }
        }
        // RTV
        {
            D3D11_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
            if(FAILED(dx.device->CreateRenderTargetView(output_texture.Get(), &desc, output_rtv.GetAddressOf())))
            {
                throw std::runtime_error("CreateRenderTargetView output texture failed");
            }
        }
        // SRV
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = 0;
            desc.Texture2D.MipLevels = 1;
            if(FAILED(dx.device->CreateShaderResourceView(output_texture.Get(), &desc, output_srv.GetAddressOf())))
            {
                throw std::runtime_error("CreateShaderResourceView output texture failed.");
            }
        }
        // DSV
        {
            if(FAILED(dx.device->CreateDepthStencilView(output_depth_texture.Get(), nullptr, output_dsv.GetAddressOf())))
            {
                throw std::runtime_error("CreateDepthStencilView output depth texture failed");
            }
        }
    }

    void rgpu::init()
    {
        rrenderer_base::init();
        
        // FIX temporary hack here! It should be properly persistent as part for the scene (not hittable)
        vertex_shader_asset.set_name("gpu_renderer_vs");
        vertex_shader_asset.get();
        pixel_shader_asset.set_name("gpu_renderer_ps");
        pixel_shader_asset.get();
        texture_asset.set_name("default");
        texture_asset.get();
        //mesh_asset.set_name("icosphere2");
        //mesh_asset.get();

        auto device = fdx11::instance().device;

        // Input layout
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
            const auto blob = vertex_shader_asset.get()->shader_blob;
            if(FAILED(device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout)))
            {
                throw std::runtime_error("CreateInputLayout failed.");
            }
        }

        // Poke static mesh loading
        for (const hhittable_base* obj : scene->objects)
        {
            if (obj->get_class() == hstatic_mesh::get_class_static())
            {
                const hstatic_mesh* sm = static_cast<const hstatic_mesh*>(obj);
                const astatic_mesh* sma = sm->mesh_asset_ptr.get();
                assert(sma->render_state.index_buffer);
                assert(sma->render_state.vertex_buffer);
            }
        }

        // Sampler state
        {
            D3D11_SAMPLER_DESC desc = {};
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
            desc.BorderColor[0] = 1.0f;
            desc.BorderColor[1] = 1.0f;
            desc.BorderColor[2] = 1.0f;
            desc.BorderColor[3] = 1.0f;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

            if(FAILED(device->CreateSamplerState(&desc, &sampler_state)))
            {
                throw std::runtime_error("CreateSamplerState failed.");
            }
        }

        // Texture asset
        {
            const atexture* temp = texture_asset.get();
            int texture_bytes_per_row = 0;
            const void* texture_bytes = nullptr;
            if (temp->is_hdr)
            {
                texture_bytes = static_cast<const void*>(temp->data_hdr);
                texture_bytes_per_row = temp->desired_channels * sizeof(float) * temp->width;
            }
            else
            {
                texture_bytes = static_cast<const void*>(temp->data_ldr);
                texture_bytes_per_row = temp->desired_channels * sizeof(uint8_t) * temp->width;
            }

            D3D11_TEXTURE2D_DESC texture_desc = {};
            texture_desc.Width = temp->width;
            texture_desc.Height = temp->height;
            texture_desc.MipLevels = 1;
            texture_desc.ArraySize = 1;
            texture_desc.Format = temp->is_hdr ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
            texture_desc.SampleDesc.Count = 1;
            texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
            texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA texture_subresource_data = {};
            texture_subresource_data.SysMemPitch = texture_bytes_per_row;
            texture_subresource_data.pSysMem = texture_bytes;

            ComPtr<ID3D11Texture2D> texture;
            if(FAILED(device->CreateTexture2D(&texture_desc, &texture_subresource_data, texture.GetAddressOf())))
            {
                throw std::runtime_error("CreateTexture2D texture asset failed.");
            }

            if(FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, texture_srv.GetAddressOf())))
            {
                throw std::runtime_error("CreateShaderResourceView texture asset failed.");
            }
        }

        // Constant buffers
        {
            D3D11_BUFFER_DESC desc = {};
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            desc.ByteWidth = sizeof(fobject_data);
            if(FAILED(device->CreateBuffer(&desc, nullptr, &object_constant_buffer)))
            {
                throw std::runtime_error("CreateBuffer object constant buffer failed.");
            }

            desc.ByteWidth = sizeof(fframe_data);
            if(FAILED(device->CreateBuffer(&desc, nullptr, &frame_constant_buffer)))
            {
                throw std::runtime_error("CreateBuffer frame constant buffer failed.");
            }
            
            // TODO - remove?
            //desc.ByteWidth      = sizeof(fmaterial_properties);
            //result = device->CreateBuffer(&desc, nullptr, &material_constant_buffer);
            //assert(SUCCEEDED(result));
            //
            //desc.ByteWidth      = sizeof(flight_properties);
            //result = device->CreateBuffer(&desc, nullptr, &light_constant_buffer);
            //assert(SUCCEEDED(result));
        }

        // Rasterizer state
        {
            D3D11_RASTERIZER_DESC desc = {};
            desc.AntialiasedLineEnable = FALSE;
            desc.CullMode = D3D11_CULL_BACK;
            desc.DepthBias = 0;
            desc.DepthBiasClamp = 0.0f;
            desc.DepthClipEnable = TRUE;
            desc.FillMode = D3D11_FILL_SOLID;
            desc.FrontCounterClockwise = FALSE;
            desc.MultisampleEnable = FALSE;
            desc.ScissorEnable = FALSE;
            desc.SlopeScaledDepthBias = 0.0f;
            if(FAILED(device->CreateRasterizerState(&desc, &rasterizer_state)))
            {
                throw std::runtime_error("CreateRasterizerState failed.");
            }
        }

        // Depth stencil state
        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = TRUE;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_LESS;
            desc.StencilEnable = FALSE;
            if(FAILED(device->CreateDepthStencilState(&desc, &depth_stencil_state)))
            {
                throw std::runtime_error("CreateDepthStencilState failed.");
            }
        }
    }

    void rgpu::render_frame(const hscene* in_scene, const frenderer_config& in_renderer_config, const fcamera_config& in_camera_config)
    {
        if (in_renderer_config.resolution_vertical == 0 || in_renderer_config.resolution_horizontal == 0) return;
        start_frame_timer();
        
        rrenderer_base::render_frame(in_scene, in_renderer_config, in_camera_config);
        
        fdx11& dx = fdx11::instance();

        dx.device_context->ClearRenderTargetView(output_rtv.Get(), Colors::LightSlateGray);
        dx.device_context->ClearDepthStencilView(output_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        const D3D11_VIEWPORT viewport = {0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f};
        dx.device_context->RSSetViewports(1, &viewport);
        dx.device_context->RSSetState(rasterizer_state.Get());
        dx.device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);
        dx.device_context->OMSetRenderTargets(1, output_rtv.GetAddressOf(), output_dsv.Get());

        dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        dx.device_context->IASetInputLayout(input_layout.Get());
        dx.device_context->VSSetShader(vertex_shader_asset.get()->shader.Get(), nullptr, 0);
        dx.device_context->VSSetConstantBuffers(0, 1, frame_constant_buffer.GetAddressOf());
        dx.device_context->VSSetConstantBuffers(1, 1, object_constant_buffer.GetAddressOf());
        dx.device_context->PSSetShader(pixel_shader_asset.get()->shader.Get(), nullptr, 0);
        dx.device_context->PSSetShaderResources(0, 1, texture_srv.GetAddressOf());
        dx.device_context->PSSetSamplers(0, 1, &sampler_state);

        // Update per frame constant buffer
        {
            fframe_data pfd;
            pfd.view_projection = camera.view_projection;
            
            D3D11_MAPPED_SUBRESOURCE data;
            dx.device_context->Map(frame_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
            *static_cast<fframe_data*>(data.pData) = pfd;
            dx.device_context->Unmap(frame_constant_buffer.Get(), 0);
        }
        
        // Draw the scene
        for (const hhittable_base* obj : scene->objects)
        {
            if (obj->get_class() == hstatic_mesh::get_class_static())
            {
                const hstatic_mesh* sm = static_cast<const hstatic_mesh*>(obj);
                const astatic_mesh* sma = sm->mesh_asset_ptr.get();
                if (sma == nullptr) { continue; }
                const fstatic_mesh_render_state& smrs = sma->render_state;
                const amaterial* ma = sm->material_asset_ptr.get();
                if (ma == nullptr) { continue; }
                
                // Object const buffer
                {
                    const XMVECTOR model_pos = XMVectorSet(sm->origin.x, sm->origin.y, sm->origin.z, 1.f);

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

                    D3D11_MAPPED_SUBRESOURCE data;
                    dx.device_context->Map(object_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
                    *static_cast<fobject_data*>(data.pData) = pod;
                    dx.device_context->Unmap(object_constant_buffer.Get(), 0);
                }

                // Draw mesh
                dx.device_context->IASetVertexBuffers(0, 1, smrs.vertex_buffer.GetAddressOf(), &smrs.stride, &smrs.offset);
                dx.device_context->IASetIndexBuffer(smrs.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                static_assert(sizeof(fface_data_type) == sizeof(uint32_t));
                dx.device_context->DrawIndexed(smrs.num_indices, 0, 0);
            }
        }
        stop_frame_timer();
    }
}
