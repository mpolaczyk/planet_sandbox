#include <d3d11_1.h>
#include <DirectXColors.h>
#include "renderer/dx11_lib.h"

#include "object/object_registry.h"
#include "renderers/gpu_renderer.h"

#include "engine/log.h"
#include "hittables/static_mesh.h"

namespace engine
{
    OBJECT_DEFINE(gpu_renderer, renderer_base, GPU renderer)
    OBJECT_DEFINE_SPAWN(gpu_renderer)

    void gpu_renderer::render_frame(const scene* in_scene, const renderer_config& in_renderer_config, const camera_config& in_camera_config)
    {
        if (in_renderer_config.resolution_vertical == 0 || in_renderer_config.resolution_horizontal == 0) return;

        camera = in_camera_config;
        scenee = in_scene;

        const bool recreate_output_buffers = output_width != in_renderer_config.resolution_vertical || output_height != in_renderer_config.resolution_horizontal;
        output_width = in_renderer_config.resolution_horizontal;
        output_height = in_renderer_config.resolution_vertical;
        if (recreate_output_buffers)
        {
            create_output_texture(true);
        }

        if (!init_done)
        {
            init();
            init_done = true;
        }

        update_frame();
    }

    void gpu_renderer::create_output_texture(bool cleanup)
    {
        if (cleanup)
        {
            DX_RELEASE(output_rtv)
            DX_RELEASE(output_srv)
            DX_RELEASE(output_texture)
        }
        dx11& dx = dx11::instance();
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
            HRESULT result = dx.device->CreateTexture2D(&desc, NULL, &output_texture);
            assert(SUCCEEDED(result));

            desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            result = dx.device->CreateTexture2D(&desc, NULL, &output_depth_texture);
            assert(SUCCEEDED(result));
        }
        {
            D3D11_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
            HRESULT result = dx.device->CreateRenderTargetView(output_texture, &desc, &output_rtv);
            assert(SUCCEEDED(result));

            result = dx.device->CreateDepthStencilView(output_depth_texture, nullptr, &output_dsv);
            assert(SUCCEEDED(result));
        }
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = 0;
            desc.Texture2D.MipLevels = 1;
            HRESULT result = dx.device->CreateShaderResourceView(output_texture, &desc, &output_srv);
            assert(SUCCEEDED(result));
        }
    }

    void gpu_renderer::init()
    {
        // FIX temporary hack here! It should be properly persistent as part for the scene (not hittable)
        vertex_shader_asset.set_name("gpu_renderer_vs");
        vertex_shader_asset.get();
        pixel_shader_asset.set_name("gpu_renderer_ps");
        pixel_shader_asset.get();
        texture_asset.set_name("default");
        texture_asset.get();
        //mesh_asset.set_name("icosphere2");
        //mesh_asset.get();

        create_output_texture();

        auto device = dx11::instance().device;

        // Measuring time
        {
            timestamp_start = 0;
            perf_counter_frequency = 0;
            {
                LARGE_INTEGER perf_count;
                QueryPerformanceCounter(&perf_count);
                timestamp_start = perf_count.QuadPart;
                LARGE_INTEGER perf_freq;
                QueryPerformanceFrequency(&perf_freq);
                perf_counter_frequency = perf_freq.QuadPart;
            }
            current_time = 0.0;
        }

        // Input layout
        {
            D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
            {
                // Per-vertex
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };
            const auto blob = vertex_shader_asset.get()->shader_blob;
            HRESULT result = device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc), blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout);
            assert(SUCCEEDED(result));
        }

        // Poke static mesh loading
        for (const hittable* obj : scenee->objects)
        {
            if (obj->get_class() == static_mesh::get_class_static())
            {
                const static_mesh* sm = static_cast<const static_mesh*>(obj);
                const static_mesh_asset* sma = sm->mesh_asset_ptr.get();
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

            HRESULT result = device->CreateSamplerState(&desc, &sampler_state);
            assert(SUCCEEDED(result));
        }

        // Texture asset
        {
            const engine::texture_asset* temp = texture_asset.get();
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

            ID3D11Texture2D* texture;
            HRESULT result = device->CreateTexture2D(&texture_desc, &texture_subresource_data, &texture);
            assert(SUCCEEDED(result));

            result = device->CreateShaderResourceView(texture, nullptr, &texture_srv);
            assert(SUCCEEDED(result));
            texture->Release();
        }

        // Constant buffers
        {
            D3D11_BUFFER_DESC desc = {};
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            desc.ByteWidth = sizeof(per_object_data);
            HRESULT result = device->CreateBuffer(&desc, nullptr, &per_object_constant_buffer);
            assert(SUCCEEDED(result));

            desc.ByteWidth = sizeof(per_frame_data);
            result = device->CreateBuffer(&desc, nullptr, &per_frame_constant_buffer);
            assert(SUCCEEDED(result));
            
            // TODO - remove?
            //desc.ByteWidth      = sizeof(material_properties);
            //result = device->CreateBuffer(&desc, nullptr, &material_constant_buffer);
            //assert(SUCCEEDED(result));
            //
            //desc.ByteWidth      = sizeof(light_properties);
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
            HRESULT result = device->CreateRasterizerState(&desc, &rasterizer_state);
            assert(SUCCEEDED(result));
        }

        // Depth stencil state
        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = TRUE;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_LESS;
            desc.StencilEnable = FALSE;
            HRESULT result = device->CreateDepthStencilState(&desc, &depth_stencil_state);
            assert(SUCCEEDED(result));
        }
    }

    void gpu_renderer::update_frame()
    {
        // Time flow
        float delta_time = 0.0f;
        {
            const double previous_time = current_time;
            LARGE_INTEGER perf_count;
            QueryPerformanceCounter(&perf_count);
            const LONGLONG timestamp_now = perf_count.QuadPart;
            current_time = static_cast<double>(timestamp_now - timestamp_start) / static_cast<double>(perf_counter_frequency);
            delta_time = static_cast<float>(current_time - previous_time);
        }

        // Camera and MVP matrix
        const float field_of_view = XMConvertToRadians(camera.field_of_view);
        const float aspect_ratio = camera.aspect_ratio_w / camera.aspect_ratio_h;
        const XMMATRIX projection = XMMatrixPerspectiveFovRH(field_of_view, aspect_ratio, math::t_min, math::infinity);

        const XMVECTOR camera_pos = XMVectorSet(camera.look_from.x, camera.look_from.y, camera.look_from.z, 0.f);
        const XMVECTOR camera_rot = XMVector3AngleBetweenVectors(XMVectorSet(0.f, 0.f, -1.f, 0.f), XMVectorSet(camera.look_dir.x, camera.look_dir.y, camera.look_dir.z, 0.f));
        const XMMATRIX view = XMMatrixMultiply(XMMatrixTranslationFromVector(camera_pos), XMMatrixRotationRollPitchYawFromVector(XMVectorScale(camera_rot, -1.f)));
        // Negate the rotation to achieve inverse
        // FIX rotation does not work well this way, test with mouse!
        const XMMATRIX view_projection = XMMatrixMultiply(view, projection);

        dx11& dx = dx11::instance();

        dx.device_context->ClearRenderTargetView(output_rtv, Colors::LightSlateGray);
        dx.device_context->ClearDepthStencilView(output_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

        const D3D11_VIEWPORT viewport = {
            0.0f, 0.0f, static_cast<float>(output_width), static_cast<float>(output_height), 0.0f, 1.0f
        };
        dx.device_context->RSSetViewports(1, &viewport);
        dx.device_context->RSSetState(rasterizer_state);
        dx.device_context->OMSetDepthStencilState(depth_stencil_state, 0);
        dx.device_context->OMSetRenderTargets(1, &output_rtv, output_dsv);

        dx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        dx.device_context->IASetInputLayout(input_layout);
        dx.device_context->VSSetShader(vertex_shader_asset.get()->shader, nullptr, 0);
        dx.device_context->VSSetConstantBuffers(0, 1, &per_frame_constant_buffer);
        dx.device_context->VSSetConstantBuffers(0, 1, &per_object_constant_buffer);
        dx.device_context->PSSetShader(pixel_shader_asset.get()->shader, nullptr, 0);
        dx.device_context->PSSetShaderResources(0, 1, &texture_srv);
        dx.device_context->PSSetSamplers(0, 1, &sampler_state);

        // Update per frame constant buffer
        {
            per_frame_data pfd;
            XMStoreFloat4x4(&pfd.view_projection, view_projection);
        
            D3D11_MAPPED_SUBRESOURCE data;
            dx.device_context->Map(per_frame_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
            *static_cast<per_frame_data*>(data.pData) = pfd;
            dx.device_context->Unmap(per_frame_constant_buffer, 0);
        }
        
        // Draw the scene
        for (const hittable* obj : scenee->objects)
        {
            if (obj->get_class() == static_mesh::get_class_static())
            {
                const static_mesh* sm = static_cast<const static_mesh*>(obj);
                const static_mesh_asset* sma = sm->mesh_asset_ptr.get();
                if (sma == nullptr) { continue; }
                const static_mesh_render_state& smrs = sma->render_state;

                // Model
                const XMVECTOR model_pos = XMVectorSet(sm->origin.x, sm->origin.y, sm->origin.z, 0.f);
                const XMVECTOR model_rot = XMVectorSet(sm->rotation.x, sm->rotation.y, sm->rotation.z, 0.f);
                const XMVECTOR model_scale = XMVectorSet(sm->scale.x, sm->scale.y, sm->scale.z, 0.f);
                const XMMATRIX model_world = XMMatrixMultiply(XMMatrixScalingFromVector(model_scale), XMMatrixMultiply(XMMatrixTranslationFromVector(model_pos), XMMatrixRotationRollPitchYawFromVector(model_rot)));
                const XMMATRIX model_inverse_transpose_world = XMMatrixTranspose(XMMatrixInverse(nullptr, model_world));
                const XMMATRIX model_view_projection = XMMatrixMultiply(XMMatrixMultiply(model_world, view), projection);
                
                // Update per object constant buffer
                {
                    per_object_data pod;
                    XMStoreFloat4x4(&pod.world, model_world);
                    XMStoreFloat4x4(&pod.inverse_transpose_world, model_inverse_transpose_world);
                    XMStoreFloat4x4(&pod.world_view_projection, XMMatrixTranspose(model_view_projection)); // Transpose: row vs column

                    D3D11_MAPPED_SUBRESOURCE data;
                    dx.device_context->Map(per_object_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
                    *static_cast<per_object_data*>(data.pData) = pod;
                    dx.device_context->Unmap(per_object_constant_buffer, 0);
                }

                // Draw mesh
                dx.device_context->IASetVertexBuffers(0, 1, &smrs.vertex_buffer, &smrs.stride, &smrs.offset);
                dx.device_context->IASetIndexBuffer(smrs.index_buffer, DXGI_FORMAT_R16_UINT, 0);
                dx.device_context->DrawIndexed(smrs.num_indices, 0, 0);
            }
        }
    }

    void gpu_renderer::destroy()
    {
        DX_RELEASE(output_rtv)
        DX_RELEASE(output_dsv)
        DX_RELEASE(input_layout)
        DX_RELEASE(texture_srv)
        DX_RELEASE(sampler_state)
        DX_RELEASE(per_frame_constant_buffer)
        DX_RELEASE(per_object_constant_buffer)
        DX_RELEASE(rasterizer_state)
        DX_RELEASE(depth_stencil_state)
        renderer_base::destroy();
    }
}
