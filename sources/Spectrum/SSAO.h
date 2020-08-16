#pragma once
#ifdef _NEED_SSAO_
enum QualityLevel { kSsaoQualityVeryLow, kSsaoQualityLow, kSsaoQualityMedium, kSsaoQualityHigh, kSsaoQualityVeryHigh, kNumSsaoQualitySettings };

class NewSSAO:public Events::prop_handler
{
        Render::ComputePipelineState::ptr depth_prepare1;
        Render::ComputePipelineState::ptr depth_prepare2;
        Render::ComputePipelineState::ptr render_1;
        Render::ComputePipelineState::ptr render_2;
        Render::ComputePipelineState::ptr blur_upsample_blend[2];
        Render::ComputePipelineState::ptr blur_upsample_final[2];
        float SampleThickness[12];


        Render::PipelineState::ptr scene_add_state;

        /* Render::HandleTable table_uav;
         Render::HandleTable table_uav2;*/
        /*   Render::HandleTable table_srv[4];
        */
    public:
        Render::Texture::ptr tex_ao_full;
        Render::Texture::ptr tex_depth_downsample[4];
        Render::Texture::ptr tex_depth_tiled[4];
        Render::Texture::ptr tex_ao_merged[4];
        Render::Texture::ptr tex_ao_smooth[3];
        Render::Texture::ptr tex_ao_hq[4];

        Render::Texture::ptr tex_depth_linear;

        Render::Texture::ptr cubemap;

        G_Buffer* buffer;
        camera* cam;
        void resize(G_Buffer& buffer)
        {
         

            /* table_uav[0] = (tex_depth_linear->texture_2d()->uav());
             table_uav[1] = (tex_depth_downsample[0]->texture_2d()->uav());
             table_uav[2] = (tex_depth_tiled[0]->array2d()->uav());
             table_uav[3] = (tex_depth_downsample[1]->texture_2d()->uav());
             table_uav[4] = (tex_depth_tiled[1]->array2d()->uav());
             table_uav2[0] = (tex_depth_downsample[2]->texture_2d()->uav());
             table_uav2[1] = (tex_depth_tiled[2]->array2d()->uav());
             table_uav2[2] = (tex_depth_downsample[3]->texture_2d()->uav());
             table_uav2[3] = (tex_depth_tiled[3]->array2d()->uav());*/
            /*table_srv[0][0] = (tex_depth_downsample[3]->texture_2d()->srv());
            table_srv[0][1] = (tex_depth_downsample[2]->texture_2d()->srv());
            table_srv[1][0] = (tex_depth_downsample[2]->texture_2d()->srv());
            table_srv[1][1] = (tex_depth_downsample[1]->texture_2d()->srv());
            table_srv[2][0] = (tex_depth_downsample[1]->texture_2d()->srv());
            table_srv[2][1] = (tex_depth_downsample[0]->texture_2d()->srv());
            table_srv[3][0] = (tex_depth_downsample[0]->texture_2d()->srv());
            table_srv[3][1] = (tex_depth_linear->texture_2d()->srv());
            table_srv[0][2] = (tex_ao_merged[3]->texture_2d()->srv());
            table_srv[0][3] = (tex_ao_hq[3]->texture_2d()->srv());
            table_srv[0][4] = (tex_ao_merged[2]->texture_2d()->srv());
            table_srv[1][2] = (tex_ao_smooth[2]->texture_2d()->srv());
            table_srv[1][3] = (tex_ao_hq[2]->texture_2d()->srv());
            table_srv[1][4] = (tex_ao_merged[1]->texture_2d()->srv());
            table_srv[2][2] = (tex_ao_smooth[1]->texture_2d()->srv());
            table_srv[2][3] = (tex_ao_hq[1]->texture_2d()->srv());
            table_srv[2][4] = (tex_ao_merged[0]->texture_2d()->srv());
            table_srv[3][2] = (tex_ao_smooth[0]->texture_2d()->srv());
            table_srv[3][3] = (tex_ao_hq[0]->texture_2d()->srv());*/
        }
        NewSSAO(G_Buffer& buffer)
        {

			this->buffer = &buffer;
			buffer.size.register_change(this, [this](const ivec2& size) {
			
				ivec2 sizes[6];

				for (int i = 0; i < 6; i++)
				{
					int p = pow(2, i + 1);
					sizes[i].x = (size.x + p - 1) / p;
					sizes[i].y = (size.y + p - 1) / p;
				}

				tex_ao_full.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));
				tex_depth_linear.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));

				for (int i = 0; i < 4; i++)
				{
					tex_depth_downsample[i].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, sizes[i].x, sizes[i].y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));
					tex_ao_merged[i].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, sizes[i].x, sizes[i].y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));
					tex_ao_hq[i].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, sizes[i].x, sizes[i].y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));
					tex_depth_tiled[i].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, sizes[i + 2].x, sizes[i + 2].y, 16, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));

					if (i < 3)
						tex_ao_smooth[i].reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, sizes[i].x, sizes[i].y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::COMMON));
				}
			});

		
            auto t = CounterManager::get().start_count<NewSSAO>();
            Render::RootSignatureDesc root_desc;
            root_desc[0] = Render::DescriptorConstants(0, 4);
            root_desc[1] = Render::DescriptorConstBuffer(1);
            root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 5);
            root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 5);
            root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 5, 2);
            root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 3, 1);
            // root_desc[6] = Render::DescriptorConstBuffer(2);
            root_desc.set_sampler(0, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerLinearClampDesc);
            root_desc.set_sampler(1, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerLinearBorderDesc);
            auto signature = std::make_shared< Render::RootSignature>(root_desc, D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE);
            depth_prepare1.reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoPrepareDepthBuffers1CS.hlsl", "main", 0, {} }) }));
            depth_prepare2.reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoPrepareDepthBuffers2CS.hlsl", "main", 0, {} }) }));
            render_1.reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoRender1CS.hlsl", "main", 0, {} }) }));
            render_2.reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoRender2CS.hlsl", "main", 0, {} }) }));
            blur_upsample_blend[0].reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoBlurUpsampleBlendOutCS.hlsl", "main", 0, {} }) }));
            blur_upsample_blend[1].reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoBlurUpsamplePreMinBlendOutCS.hlsl", "main", 0, {} }) }));
            blur_upsample_final[0].reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoBlurUpsampleCS.hlsl", "main", 0, {} }) }));
            blur_upsample_final[1].reset(new  Render::ComputePipelineState({ signature, Render::compute_shader::get_resource({ "shaders\\AO\\AoBlurUpsamplePreMinCS.hlsl", "main", 0, {} }) }));
            Render::PipelineStateDesc desc;
            desc.root_signature = signature;
            desc.blend.render_target[0].enabled = true;
            desc.blend.render_target[0].dest = D3D12_BLEND_ONE;
            desc.blend.render_target[0].source = D3D12_BLEND_ONE;
            desc.rtv.rtv_formats = {DXGI_FORMAT_R16G16B16A16_FLOAT };
            desc.pixel = Render::pixel_shader::get_resource({ "shaders\\AO\\result.hlsl", "PS", 0, {} });
            desc.vertex = Render::vertex_shader::get_resource({ "shaders\\AO\\result.hlsl", "VS", 0, {} });
            scene_add_state.reset(new  Render::PipelineState(desc));
            SampleThickness[0] = sqrt(1.0f - 0.2f * 0.2f);
            SampleThickness[1] = sqrt(1.0f - 0.4f * 0.4f);
            SampleThickness[2] = sqrt(1.0f - 0.6f * 0.6f);
            SampleThickness[3] = sqrt(1.0f - 0.8f * 0.8f);
            SampleThickness[4] = sqrt(1.0f - 0.2f * 0.2f - 0.2f * 0.2f);
            SampleThickness[5] = sqrt(1.0f - 0.2f * 0.2f - 0.4f * 0.4f);
            SampleThickness[6] = sqrt(1.0f - 0.2f * 0.2f - 0.6f * 0.6f);
            SampleThickness[7] = sqrt(1.0f - 0.2f * 0.2f - 0.8f * 0.8f);
            SampleThickness[8] = sqrt(1.0f - 0.4f * 0.4f - 0.4f * 0.4f);
            SampleThickness[9] = sqrt(1.0f - 0.4f * 0.4f - 0.6f * 0.6f);
            SampleThickness[10] = sqrt(1.0f - 0.4f * 0.4f - 0.8f * 0.8f);
            SampleThickness[11] = sqrt(1.0f - 0.6f * 0.6f - 0.6f * 0.6f);
            /* table_uav = Render::DescriptorHeapManager::get().get_csu()->create_table(5);
             table_uav2 = Render::DescriptorHeapManager::get().get_csu()->create_table(4);*/
            /*    for (int i = 0; i < 4; i++)
                    table_srv[i] = Render::DescriptorHeapManager::get().get_csu()->create_table(5);*/
        }




        void compute_ao(DX12::ComputeContext& compute, Render::Texture::ptr& Destination, Render::Texture::ptr& DepthBuffer, const float TanHalfFovH)
        {
            size_t BufferWidth = DepthBuffer->get_desc().Width;
            size_t BufferHeight = DepthBuffer->get_desc().Height;
            size_t ArrayCount = DepthBuffer->get_desc().ArraySize();
            // Here we compute multipliers that convert the center depth value into (the reciprocal of)
            // sphere thicknesses at each sample location.  This assumes a maximum sample radius of 5
            // units, but since a sphere has no thickness at its extent, we don't need to sample that far
            // out.  Only samples whole integer offsets with distance less than 25 are used.  This means
            // that there is no sample at (3, 4) because its distance is exactly 25 (and has a thickness of 0.)
            // The shaders are set up to sample a circular region within a 5-pixel radius.
            const float ScreenspaceDiameter = 10.0f;
            // SphereDiameter = CenterDepth * ThicknessMultiplier.  This will compute the thickness of a sphere centered
            // at a specific depth.  The ellipsoid scale can stretch a sphere into an ellipsoid, which changes the
            // characteristics of the AO.
            // TanHalfFovH:  Radius of sphere in depth units if its center lies at Z = 1
            // ScreenspaceDiameter:  Diameter of sample sphere in pixel units
            // ScreenspaceDiameter / BufferWidth:  Ratio of the screen width that the sphere actually covers
            // Note about the "2.0f * ":  Diameter = 2 * Radius
            float ThicknessMultiplier = 2.0f * TanHalfFovH * ScreenspaceDiameter / BufferWidth;

            if (ArrayCount == 1)
                ThicknessMultiplier *= 2.0f;

            // This will transform a depth value from [0, thickness] to [0, 1].
            float InverseRangeFactor = 1.0f / ThicknessMultiplier;
            __declspec(align(16)) float SsaoCB[28];
            // The thicknesses are smaller for all off-center samples of the sphere.  Compute thicknesses relative
            // to the center sample.
            SsaoCB[0] = InverseRangeFactor / SampleThickness[0];
            SsaoCB[1] = InverseRangeFactor / SampleThickness[1];
            SsaoCB[2] = InverseRangeFactor / SampleThickness[2];
            SsaoCB[3] = InverseRangeFactor / SampleThickness[3];
            SsaoCB[4] = InverseRangeFactor / SampleThickness[4];
            SsaoCB[5] = InverseRangeFactor / SampleThickness[5];
            SsaoCB[6] = InverseRangeFactor / SampleThickness[6];
            SsaoCB[7] = InverseRangeFactor / SampleThickness[7];
            SsaoCB[8] = InverseRangeFactor / SampleThickness[8];
            SsaoCB[9] = InverseRangeFactor / SampleThickness[9];
            SsaoCB[10] = InverseRangeFactor / SampleThickness[10];
            SsaoCB[11] = InverseRangeFactor / SampleThickness[11];
            // These are the weights that are multiplied against the samples because not all samples are
            // equally important.  The farther the sample is from the center location, the less they matter.
            // We use the thickness of the sphere to determine the weight.  The scalars in front are the number
            // of samples with this weight because we sum the samples together before multiplying by the weight,
            // so as an aggregate all of those samples matter more.  After generating this table, the weights
            // are normalized.
            SsaoCB[12] = 4.0f * SampleThickness[0];	// Axial
            SsaoCB[13] = 4.0f * SampleThickness[1];	// Axial
            SsaoCB[14] = 4.0f * SampleThickness[2];	// Axial
            SsaoCB[15] = 4.0f * SampleThickness[3];	// Axial
            SsaoCB[16] = 4.0f * SampleThickness[4];	// Diagonal
            SsaoCB[17] = 8.0f * SampleThickness[5];	// L-shaped
            SsaoCB[18] = 8.0f * SampleThickness[6];	// L-shaped
            SsaoCB[19] = 8.0f * SampleThickness[7];	// L-shaped
            SsaoCB[20] = 4.0f * SampleThickness[8];	// Diagonal
            SsaoCB[21] = 8.0f * SampleThickness[9];	// L-shaped
            SsaoCB[22] = 8.0f * SampleThickness[10];	// L-shaped
            SsaoCB[23] = 4.0f * SampleThickness[11];	// Diagonal
            //#define SAMPLE_EXHAUSTIVELY
            // If we aren't using all of the samples, delete their weights before we normalize.
#ifndef SAMPLE_EXHAUSTIVELY
            SsaoCB[12] = 0.0f;
            SsaoCB[14] = 0.0f;
            SsaoCB[17] = 0.0f;
            SsaoCB[19] = 0.0f;
            SsaoCB[21] = 0.0f;
#endif
            float RejectionFalloff = 2.5f;
            float Accentuation = 0;
            // Normalize the weights by dividing by the sum of all weights
            float totalWeight = 0.0f;

            for (int i = 12; i < 24; ++i)
                totalWeight += SsaoCB[i];

            for (int i = 12; i < 24; ++i)
                SsaoCB[i] /= totalWeight;

            SsaoCB[24] = 1.0f / BufferWidth;
            SsaoCB[25] = 1.0f / BufferHeight;
            SsaoCB[26] = 1.0f / -RejectionFalloff;
            SsaoCB[27] = 1.0f / (1.0f + Accentuation);
            compute.set_const_buffer_raw(1, SsaoCB, sizeof(SsaoCB));  //.SetDynamicConstantBufferView(1, sizeof(SsaoCB), SsaoCB);
            compute.set_dynamic(2, 0, Destination->texture_2d()->get_static_uav());
            compute.set_dynamic(3, 0, ArrayCount == 1 ? DepthBuffer->texture_2d()->get_static_srv() : DepthBuffer->array2d()->get_static_srv());

            if (ArrayCount == 1)
                compute.dispach(ivec2(BufferWidth, BufferHeight), { 16, 16 });
            else
                compute.dispach(ivec3(BufferWidth, BufferHeight, ArrayCount), { 8, 8, 1 });
        }




        void BlurAndUpsample(DX12::ComputeContext& Context,
                             Render::Texture::ptr& Destination, Render::Texture::ptr& HiResDepth, Render::Texture::ptr& LoResDepth,
                             Render::Texture::ptr* InterleavedAO, Render::Texture::ptr* HighQualityAO, Render::Texture::ptr* HiResAO)
        {
            //		auto &Context = BaseContext.get_compute();
            float g_BlurTolerance = -5.0f;
            float g_UpsampleTolerance = -7.0f;
            float g_NoiseFilterTolerance = -3.0f;
            size_t LoWidth = LoResDepth->get_desc().Width;
            size_t LoHeight = LoResDepth->get_desc().Height;
            size_t HiWidth = HiResDepth->get_desc().Width;
            size_t HiHeight = HiResDepth->get_desc().Height;

            if (HiResAO == nullptr)
                Context.set_pipeline(blur_upsample_final[HighQualityAO == nullptr ? 0 : 1]);
            else
                Context.set_pipeline(blur_upsample_blend[HighQualityAO == nullptr ? 0 : 1]);

            float kBlurTolerance = 1.0f - powf(10.0f, g_BlurTolerance) * 1920.0f / (float)LoWidth;
            kBlurTolerance *= kBlurTolerance;
            float kUpsampleTolerance = powf(10.0f, g_UpsampleTolerance);
            float kNoiseFilterWeight = 1.0f / (powf(10.0f, g_NoiseFilterTolerance) + kUpsampleTolerance);
            __declspec(align(16)) float cbData[] =
            {
                1.0f / LoWidth, 1.0f / LoHeight, 1.0f / HiWidth, 1.0f / HiHeight,
                kNoiseFilterWeight, 1920.0f / (float)LoWidth, kBlurTolerance, kUpsampleTolerance
            };
            Context.set_const_buffer_raw(1, cbData, sizeof(cbData));
            Context.get_base().transition(Destination, Render::ResourceState::UNORDERED_ACCESS);
            Context.get_base().transition(LoResDepth, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
            Context.get_base().transition(HiResDepth, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
            Context.set_dynamic(2, 0, Destination->texture_2d()->get_static_uav());
            Context.set_dynamic(3, 0, LoResDepth->texture_2d()->get_static_srv());
            Context.set_dynamic(3, 1, HiResDepth->texture_2d()->get_static_srv());

            if (InterleavedAO != nullptr)
            {
                Context.get_base().transition(*InterleavedAO, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
                Context.set_dynamic(3, 2, (*InterleavedAO)->texture_2d()->get_static_srv());
            }

            if (HighQualityAO != nullptr)
            {
                Context.get_base().transition(*HighQualityAO, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
                Context.set_dynamic(3, 3, (*HighQualityAO)->texture_2d()->get_static_srv());
            }

            if (HiResAO != nullptr)
            {
                Context.get_base().transition(*HiResAO, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
                Context.set_dynamic(3, 4, (*HiResAO)->texture_2d()->get_static_srv());
            }

            /*  else
              {
                  Context.transition(buffer->result_tex.first(), Render::ResourceState::UNORDERED_ACCESS);
                  Context.set_dynamic(2, 1, buffer->result_tex.first()->texture_2d()->get_static_uav());
                  Context.transition(buffer->albedo_tex, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
                  Context.set_dynamic(5, 0, buffer->albedo_tex->texture_2d()->get_static_srv());
                  Context.transition(buffer->normal_tex, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
                  Context.set_dynamic(5, 1, buffer->normal_tex->texture_2d()->get_static_srv());
                  Context.transition(cubemap, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
                  Context.set_dynamic(5, 2, cubemap->cubemap()->get_static_srv());
                  Context.set_const_buffer(6, cam->get_const_buffer());
              }
            */
            //     Context.set(3, table_srv[index]);
            Context.dispach(ivec2(HiWidth + 2, HiHeight + 2), { 16, 16 });
        }






        void Render(Render::CommandList& BaseContext, Render::Texture::ptr cubemap, const mat4x4& proj, float NearClipDist, float FarClipDist)
        {
            this->cubemap = cubemap;
            auto timer = BaseContext.start(L"SSAO");
            int HierarchyDepth = 2;
            QualityLevel g_QualityLevel = kSsaoQualityVeryHigh;
            DX12::ComputeContext& Context = BaseContext.get_compute();
            Context.set_pipeline(depth_prepare1);
            const float zMagic = (FarClipDist - NearClipDist) / NearClipDist;
            {
                auto timer = BaseContext.start(L"Decompress and downsample");
                //   ScopedTimer _prof(L"Decompress and downsample", Context);
                // Phase 1:  Decompress, linearize, downsample, and deinterleave the depth buffer
                {
                    auto timer = BaseContext.start(L"transitions");
					BaseContext.transition(tex_ao_full, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
					BaseContext.transition(tex_depth_linear, Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_downsample[0], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_tiled[0], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_downsample[1], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_tiled[1], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.flush_transitions();
                }
                {
                    auto timer = BaseContext.start(L"constants");
                    Context.set_constants(0, NearClipDist, FarClipDist);
                    //   Context.set(2, table_uav);
                    Context.set_dynamic(2, 0, tex_depth_linear->texture_2d()->get_static_uav());
                    Context.set_dynamic(2, 1, tex_depth_downsample[0]->texture_2d()->get_static_uav());
                    Context.set_dynamic(2, 2, tex_depth_tiled[0]->array2d()->get_static_uav());
                    Context.set_dynamic(2, 3, tex_depth_downsample[1]->texture_2d()->get_static_uav());
                    Context.set_dynamic(2, 4, tex_depth_tiled[1]->array2d()->get_static_uav());
                    Context.set_dynamic(3, 0, buffer->depth_tex->texture_2d()->get_static_srv());
                    Context.set_pipeline(depth_prepare1);
                }
                {
                    auto timer = BaseContext.start(L"dispatch");
                    Context.dispach(ivec2(tex_depth_tiled[1]->get_desc().Width * 8, tex_depth_tiled[1]->get_desc().Height * 8));
                }

                if (HierarchyDepth > 2)
                {
                    Context.set_constants(0, 1.0f / tex_depth_downsample[1]->get_desc().Width, 1.0f / tex_depth_downsample[1]->get_desc().Height);
					BaseContext.transition(tex_depth_downsample[1], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
					BaseContext.transition(tex_depth_downsample[2], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_tiled[2], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_downsample[3], Render::ResourceState::UNORDERED_ACCESS);
					BaseContext.transition(tex_depth_tiled[3], Render::ResourceState::UNORDERED_ACCESS);
                    // Context.set(2, table_uav2);
                    Context.set_dynamic(2, 0, tex_depth_downsample[2]->texture_2d()->get_static_uav());
                    Context.set_dynamic(2, 1, tex_depth_tiled[2]->array2d()->get_static_uav());
                    Context.set_dynamic(2, 2, tex_depth_downsample[3]->texture_2d()->get_static_uav());
                    Context.set_dynamic(2, 3, tex_depth_tiled[3]->array2d()->get_static_uav());
                    Context.set_dynamic(3, 0, tex_depth_downsample[1]->texture_2d()->get_static_srv());
                    Context.set_pipeline(depth_prepare2);
                    Context.dispach(ivec2(tex_depth_tiled[3]->get_desc().Width * 8, tex_depth_tiled[3]->get_desc().Height * 8));
                }
            } // End decompress
            {
                auto timer = BaseContext.start(L"Analyze depth volumes");
                //   ScopedTimer _prof(L"Analyze depth volumes", Context);
                // Load first element of projection matrix which is the cotangent of the horizontal FOV divided by 2.
                const float FovTangent = 1.0f / proj.a11;
				BaseContext.transition(tex_ao_merged[0], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_merged[1], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_merged[2], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_merged[3], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_hq[0], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_hq[1], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_hq[2], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_ao_hq[3], Render::ResourceState::UNORDERED_ACCESS);
				BaseContext.transition(tex_depth_tiled[0], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_tiled[1], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_tiled[2], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_tiled[3], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_downsample[0], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_downsample[1], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_downsample[2], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				BaseContext.transition(tex_depth_downsample[3], Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

                // Phase 2:  Render SSAO for each sub-tile
                if (HierarchyDepth > 3)
                {
                    Context.set_pipeline(render_1);
                    compute_ao(Context, tex_ao_merged[3], tex_depth_tiled[3], FovTangent);

                    if (g_QualityLevel >= kSsaoQualityLow)
                    {
                        Context.set_pipeline(render_2);
                        compute_ao(Context, tex_ao_hq[3], tex_depth_downsample[3], FovTangent);
                    }
                }

                if (HierarchyDepth > 2)
                {
                    Context.set_pipeline(render_1);
                    compute_ao(Context, tex_ao_merged[2], tex_depth_tiled[2], FovTangent);

                    if (g_QualityLevel >= kSsaoQualityMedium)
                    {
                        Context.set_pipeline(render_2);
                        compute_ao(Context, tex_ao_hq[2], tex_depth_downsample[2], FovTangent);
                    }
                }

                if (HierarchyDepth > 1)
                {
                    Context.set_pipeline(render_1);
                    compute_ao(Context, tex_ao_merged[1], tex_depth_tiled[1], FovTangent);

                    if (g_QualityLevel >= kSsaoQualityHigh)
                    {
                        Context.set_pipeline(render_2);
                        compute_ao(Context, tex_ao_hq[1], tex_depth_downsample[1], FovTangent);
                    }
                }

                {
                    Context.set_pipeline(render_1);
                    compute_ao(Context, tex_ao_merged[0], tex_depth_tiled[0], FovTangent);

                    if (g_QualityLevel >= kSsaoQualityVeryHigh)
                    {
                        Context.set_pipeline(render_2);
                        compute_ao(Context, tex_ao_hq[0], tex_depth_downsample[0], FovTangent);
                    }
                }
            } // End analyze
            {
                auto timer = BaseContext.start(L"Blur and upsample");
                //     ScopedTimer _prof(L"Blur and upsample", Context);
                // Phase 4:  Iteratively blur and upsample, combining each result
                Render::Texture::ptr* NextSRV = &tex_ao_merged[3];

                // 120 x 68 -> 240 x 135
                if (HierarchyDepth > 3)
                {
                    BlurAndUpsample(Context, tex_ao_smooth[2], tex_depth_downsample[2], tex_depth_downsample[3], NextSRV,
                                    g_QualityLevel >= kSsaoQualityLow ? &tex_ao_hq[3] : nullptr, &tex_ao_merged[2]);
                    NextSRV = &tex_ao_smooth[2];
                }

                else
                    NextSRV = &tex_ao_merged[2];

                // 240 x 135 -> 480 x 270
                if (HierarchyDepth > 2)
                {
                    BlurAndUpsample(Context, tex_ao_smooth[1], tex_depth_downsample[1], tex_depth_downsample[2], NextSRV,
                                    g_QualityLevel >= kSsaoQualityMedium ? &tex_ao_hq[2] : nullptr  , &tex_ao_merged[1]);
                    NextSRV = &tex_ao_smooth[1];
                }

                else
                    NextSRV = &tex_ao_merged[1];

                // 480 x 270 -> 960 x 540
                if (HierarchyDepth > 1)
                {
                    BlurAndUpsample(Context, tex_ao_smooth[0], tex_depth_downsample[0], tex_depth_downsample[1], NextSRV,
                                    g_QualityLevel >= kSsaoQualityHigh ? &tex_ao_hq[1] : nullptr  , &tex_ao_merged[0]);
                    NextSRV = &tex_ao_smooth[0];
                }

                else
                    NextSRV = &tex_ao_merged[0];

                // 960 x 540 -> 1920 x 1080
                BlurAndUpsample(Context, tex_ao_full, tex_depth_linear, tex_depth_downsample[0], NextSRV,
                                g_QualityLevel >= kSsaoQualityVeryHigh ? &tex_ao_hq[0] : nullptr , nullptr);
            } // End blur and upsample
			BaseContext.transition(tex_ao_full, Render::ResourceState::PIXEL_SHADER_RESOURCE);
            {
                auto timer = BaseContext.start(L"Blending to scene");
                auto& graphics = BaseContext.get_graphics();
                graphics.set_pipeline(scene_add_state);
				BaseContext.transition(buffer->result_tex.first(), Render::ResourceState::RENDER_TARGET);
				graphics.set_viewport(buffer->result_tex.first()->texture_2d()->get_viewport());
				graphics.set_scissor(buffer->result_tex.first()->texture_2d()->get_scissor());
               // list.set_rtv(1, buffer->result_tex.first()->texture_2d()->get_rtv(), Render::Handle());
             //   list.set(3, buffer->srv_table);
              //  list.set_dynamic(4, 0, tex_ao_full->texture_2d()->get_static_srv());
              //  list.set_dynamic(4, 1, cubemap->cubemap()->get_static_srv());
               // list.set(1, cam->get_const_buffer());
          //      list.draw(4);
            }
            //	list.transition(depth_tex_mips.get(), Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
            //	list.transition(g_buffer->normal_tex.get(), Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
        }

};

#endif