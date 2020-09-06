#include "pch.h"




SkyRender::SkyRender()
{

	auto sig = get_Signature(Layouts::DefaultLayout);// <SignatureCreator>().create_root();;
	{
		Render::PipelineStateDesc desc;
		desc.root_signature = sig;

		desc.rtv.rtv_formats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.blend.render_target[0].enabled = true;
		desc.blend.render_target[0].source = D3D12_BLEND::D3D12_BLEND_ONE;
		desc.blend.render_target[0].dest = D3D12_BLEND::D3D12_BLEND_ONE;


		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\sky.hlsl", "PS", 0, {} });
		desc.vertex = Render::vertex_shader::get_resource({ "shaders\\sky.hlsl", "VS", 0, {} });
		state = Render::PipelineState::create(desc, "sky_state");
	}

	{
		Render::PipelineStateDesc state_desc;

		state_desc.root_signature = sig;

		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\sky.hlsl", "PS_Cube", 0, {} });
		state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\sky.hlsl", "VS_Cube", 0, {} });
		state_desc.rtv.rtv_formats.resize(1);
		state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		cubemap_state = Render::PipelineState::create(state_desc, "sky_cubemap_state");
	}

	transmittance = Render::Texture::get_resource({ "textures\\Transmit.dds", false, false });
	irradiance = Render::Texture::get_resource({ "textures\\irradianceTexture.dds", false, false });
	inscatter = Render::Texture::get_resource({ "textures\\inscatterTexture.dds", false, false });
}



void SkyRender::generate(FrameGraph& graph)
{

	struct SkyData
	{
		ResourceHandler* depth;

		ResourceHandler* target_tex;


		ResourceHandler* sky_cubemap;
	

	};

	graph.add_pass<SkyData>("Sky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		data.depth = builder.need_texture("GBuffer_Depth", ResourceFlags::PixelRead);
		data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
		data.sky_cubemap = builder.create_texture("sky_cubemap", ivec2(256, 256),6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget| ResourceFlags::Cube);
		}, [this, &graph](SkyData& data, FrameContext& _context) {
			auto depth = _context.get_texture(data.depth);

			auto target_tex = _context.get_texture(data.target_tex);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();

			auto sky_cubemap = _context.get_texture(data.sky_cubemap);


			graphics.set_pipeline(state);


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



			graphics.set_viewport(target_tex.get_viewport());
			graphics.set_scissor(target_tex.get_scissor());
			graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());

			{
				Slots::SkyData data;

				data.GetInscatter() = inscatter->texture_3d()->get_static_srv();
				data.GetIrradiance() = irradiance->texture_2d()->get_static_srv();
				data.GetTransmittance() = transmittance->texture_2d()->get_static_srv();

				data.GetDepthBuffer() = depth.get_srv();

				data.GetSunDir() = graph.sunDir;

				data.set(graphics);

			}
			{
				Slots::FrameInfo frameInfo;

				auto camera = frameInfo.MapCamera();
				camera.cb = graph.cam->camera_cb.current;
				//memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
				frameInfo.set(graphics);
			}

			graphics.use_dynamic = false;
			graphics.draw(4);



			{
				
				graphics.set_viewport(sky_cubemap.get_viewport());
				graphics.set_scissor(sky_cubemap.get_scissor());
				graphics.set_pipeline(cubemap_state);


				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = 0;
					subres.Texture2D.PlaneSlice = 0;

					auto face = sky_cubemap.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);


					Slots::SkyFace skyFace;

					skyFace.GetFace() = i;

					skyFace.set(graphics);

					graphics.set_rtv(1, face.get_rtv(), Render::Handle());

					graphics.draw(4);
				}
			}

	//		auto tex = std::dynamic_pointer_cast<Render::Texture>(sky_cubemap.resource);


			MipMapGenerator::get().generate_cube(list.get_compute(), sky_cubemap);

		});




	CubeMapEnviromentProcessor::get().generate(graph);

}

/*

void SkyRender::update_cubemap(MeshRenderContext::ptr& context)
{
	auto timer = context->list->start(L"cubemap");
	//	static bool processed = false;

	if (float3::dot(dir, context->sky_dir) < 0.999)
	{
		processed = false;
		dir = context->sky_dir;
	}
	if (processed) return;

	processed = true;
	auto& list = *context->list;
	auto& graphics = context->list->get_graphics();
	auto& view = cubemap->cubemap();

	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	graphics.set_pipeline(cubemap_state);
	graphics.set_viewport(view->get_viewport());
	graphics.set_scissor(view->get_scissor());
	SkySignature<Signature> shader_data(&graphics);



	shader_data.prepared_textures[0] = transmittance->texture_2d()->get_static_srv();
	shader_data.prepared_textures[1] = irradiance->texture_2d()->get_static_srv();
	shader_data.prepared_textures[2] = inscatter->texture_3d()->get_static_srv();

	shader_data.constants.set(context->sky_dir.x, context->sky_dir.y, context->sky_dir.z);
	//	list.assume_state(cubemap, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.transition(cubemap, Render::ResourceState::RENDER_TARGET);

	for (unsigned int i = 0; i < 6; i++)
	{
		graphics.set_rtv(1, view->get_rtv(i, 0), Render::Handle());
		shader_data.face_constants.set(i);
		graphics.draw(4);
	}

	MipMapGenerator::get().generate(list.get_compute(), cubemap, cubemap->cubemap());


	CubeMapEnviromentProcessor::get().process(context, cubemap, enviroment.prefiltered_cubemap);
	CubeMapEnviromentProcessor::get().process_diffuse(context, cubemap, enviroment.prefiltered_cubemap_diffuse);
	//    list.transition(cubemap, Render::ResourceState::PIXEL_SHADER_RESOURCE);
}
*/


CubeMapEnviromentProcessor::CubeMapEnviromentProcessor()
{
	Render::PipelineStateDesc state_desc;
	Render::RootSignatureDesc root_desc;
	state_desc.root_signature = get_Signature(Layouts::DefaultLayout);

	state_desc.vertex = Render::vertex_shader::get_resource({"shaders\\cubemap_down.hlsl", "VS", 0, {}});
	state_desc.rtv.rtv_formats.resize(1);
	state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
	char* counts[] = {"1", "8", "32", "64", "128"};

	for (int i = 0; i < 5; i++)
	{
		state_desc.pixel = Render::pixel_shader::get_resource({
			"shaders\\cubemap_down.hlsl", "PS", 0, {D3D::shader_macro("NumSamples", counts[i])}
		});
		states[i] =Render::PipelineState::create(state_desc,std::string("cubeenv_desc_")+std::to_string(i));
	}


	state_desc.pixel = Render::pixel_shader::get_resource({"shaders\\cubemap_down.hlsl", "PS_Diffuse", 0, {}});
	state_diffuse =Render::PipelineState::create(state_desc,"cubeenv_diffuse");
}

void CubeMapEnviromentProcessor::generate(FrameGraph &graph)
{

	struct EnvData
	{

		ResourceHandler* sky_cubemap;

		ResourceHandler* sky_cubemap_filtered;
		ResourceHandler* sky_cubemap_filtered_diffuse;

	};

	graph.add_pass<EnvData>("CubeMapEnviromentProcessor", [this, &graph](EnvData& data, TaskBuilder& builder) {
		data.sky_cubemap = builder.need_texture("sky_cubemap", ResourceFlags::PixelRead);

		data.sky_cubemap_filtered = builder.create_texture("sky_cubemap_filtered", ivec2(64, 64), 6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget | ResourceFlags::Cube);
		data.sky_cubemap_filtered_diffuse = builder.create_texture("sky_cubemap_filtered_diffuse", ivec2(64, 64), 6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget | ResourceFlags::Cube);


		}, [this, &graph](EnvData& data, FrameContext& _context) {
			auto sky_cubemap = _context.get_texture(data.sky_cubemap);
			auto sky_cubemap_filtered = _context.get_texture(data.sky_cubemap_filtered);
			auto sky_cubemap_filtered_diffuse = _context.get_texture(data.sky_cubemap_filtered_diffuse);

		
			auto& list = *_context.get_list();
			auto& graphics = list.get_graphics();

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));

			Slots::EnvSource downsample;
			downsample.GetSourceTex() = sky_cubemap.get_srv();
			downsample.set(graphics);



			UINT count = sky_cubemap_filtered.resource->get_desc().MipLevels;
			for (unsigned int m = 0; m < count; m++)
			{
				graphics.set_pipeline(states[std::min(m, 4u)]);

				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = m;
					subres.Texture2D.PlaneSlice = 0;

					auto face = sky_cubemap_filtered.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

					if (i == 0) {
						graphics.set_viewport(face.get_viewport());
						graphics.set_scissor(face.get_scissor());
					}
					graphics.set_rtv(1, face.get_rtv(), Render::Handle());

					Slots::EnvFilter filter;
					filter.GetFace().x = i;

					filter.GetScaler().x = (float(m) + 0.5f) / count;
					filter.GetSize().x = sky_cubemap.resource->get_desc().Width;
					filter.set(graphics);
					
					graphics.draw(4);
				}
			}


		
				graphics.set_pipeline(state_diffuse);

				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = 0;
					subres.Texture2D.PlaneSlice = 0;

					auto face = sky_cubemap_filtered_diffuse.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

					if (i == 0) {
						graphics.set_viewport(face.get_viewport());
						graphics.set_scissor(face.get_scissor());
					}
					graphics.set_rtv(1, face.get_rtv(), Render::Handle());

					Slots::EnvFilter filter;
					filter.GetFace().x = i;

					filter.GetScaler().x = (float(0) + 0.5f) / count;
					filter.GetSize().x = sky_cubemap.resource->get_desc().Width;
					filter.set(graphics);

					graphics.draw(4);
				
			}
		});



}

/*
void CubeMapEnviromentProcessor::process_diffuse(MeshRenderContext::ptr& context, Render::Texture::ptr cubemap,
												 Render::Texture::ptr cubemap_result)
{
	auto& list = context->list->get_graphics();


	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->list->transition(cubemap.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
	context->list->transition(cubemap_result.get(), Render::ResourceState::RENDER_TARGET);

	list.set_pipeline(state_diffuse);
	CubeEnvSignature<Signature> shader_data(&context->list->get_graphics());

	shader_data.source_tex[0] = cubemap->cubemap()->get_srv();

	auto& view = cubemap_result->cubemap();


	int m = 0;

	list.set_viewport(view->get_viewport(m));
	list.set_scissor(view->get_scissor(m));

	for (unsigned int i = 0; i < 6; i++)
	{
		context->list->get_graphics().set_rtv(1, view->get_rtv(i, m), Render::Handle());
		shader_data.constants.set(i, (float(m) + 0.5f) / cubemap->get_desc().MipLevels,
													(unsigned int)cubemap->get_desc().Width);
		list.draw(4);
	}
}
*/