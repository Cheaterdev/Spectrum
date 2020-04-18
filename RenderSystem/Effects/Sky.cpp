#include "pch.h"


template <class T>
struct CubeEnvSignature : public T
{
	using T::T;

	typename T::template Constants		<0, Render::ShaderVisibility::ALL, 0, 4>									constants = this;
	typename T::template Table			<1, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1>	source_tex = this;

	typename T::template Sampler<0, Render::ShaderVisibility::PIXEL, 0> linear{ Render::Samplers::SamplerLinearWrapDesc, this };


};



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
		state.reset(new Render::PipelineState(desc));
	}

	{
		Render::PipelineStateDesc state_desc;

		state_desc.root_signature = sig;

		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\sky.hlsl", "PS_Cube", 0, {} });
		state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\sky.hlsl", "VS_Cube", 0, {} });
		state_desc.rtv.rtv_formats.resize(1);
		state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		cubemap_state.reset(new Render::PipelineState(state_desc));
	}

	transmittance = Render::Texture::get_resource({ "textures\\Transmit.dds", false, false });
	irradiance = Render::Texture::get_resource({ "textures\\irradianceTexture.dds", false, false });
	inscatter = Render::Texture::get_resource({ "textures\\inscatterTexture.dds", false, false });

	cubemap.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, 256, 256,
		6, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

	enviroment.prefiltered_cubemap.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, 256, 256, 6, 6, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

	enviroment.prefiltered_cubemap_diffuse.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, 64, 64, 6, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

}



void SkyRender::generate(FrameGraph& graph)
{

	struct SkyData
	{
		ResourceHandler* depth;

		ResourceHandler* target_tex;
	};

	graph.add_pass<SkyData>("Sky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		data.depth = builder.need_texture("GBuffer_Depth", ResourceFlags::PixelRead);
		data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);

		}, [this, &graph](SkyData& data, FrameContext& _context) {
			auto depth = _context.get_texture(data.depth);

			auto target_tex = _context.get_texture(data.target_tex);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();




			graphics.set_pipeline(state);


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



			list.transition(target_tex.resource, Render::ResourceState::RENDER_TARGET);



			graphics.set_viewport(target_tex.get_viewport());
			graphics.set_scissor(target_tex.get_scissor());
			graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());

			{
				Slots::SkyData data;

				data.GetInscatter() = inscatter->texture_3d()->get_static_srv();
				data.GetIrradiance() = irradiance->texture_2d()->get_static_srv();
				data.GetTransmittance() = transmittance->texture_2d()->get_static_srv();

				data.GetDepthBuffer() = depth.get_srv();

				data.GetSunDir() = { 0,1,0 };

				data.set(graphics);

			}
			{
				Slots::FrameInfo frameInfo;

				auto camera = frameInfo.MapCamera();
				memcpy(&camera.cb, &graph.cam->get_raw_cb().current, sizeof(camera.cb));
				frameInfo.set(graphics);
			}

			graphics.use_dynamic = false;
			graphics.draw(4);

		});




}
/*

void SkyRender::process(MeshRenderContext::ptr& context)
{
	auto timer = context->list->start(L"sky");

	auto& list = *context->list;
	auto& graphics = context->list->get_graphics();

	graphics.set_pipeline(state);



	//SkySignature<Signature> shader_data(&graphics);

	shader_data.constants.set(context->sky_dir.x, context->sky_dir.y, context->sky_dir.z);
	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	for (auto& e : context->eye_context->eyes)
	{
		auto g_buffer = e.g_buffer;
		auto cam = e.cam;
		list.transition(g_buffer->result_tex.first(), Render::ResourceState::RENDER_TARGET);
	}

	for (auto& e : context->eye_context->eyes)
	{
		auto g_buffer = e.g_buffer;
		auto cam = e.cam;

		auto& view = g_buffer->result_tex.first()->texture_2d();
		graphics.set_viewport(view->get_viewport());
		graphics.set_scissor(view->get_scissor());
		graphics.set_rtv(1, view->get_rtv(), Render::Handle());

		{
			Slots::SkyData data;

			data.GetInscatter() = inscatter->texture_3d()->get_static_srv();
			data.GetIrradiance() = irradiance->texture_2d()->get_static_srv();
			data.GetTransmittance() = transmittance->texture_3d()->get_static_srv();

			data.GetDepthBuffer() = g_buffer->depth->get_srv();

			data.GetSkyDir() = context->sky_dir;

			data.set(graphics);

		}
		{
			Slots::FrameInfo frameInfo;

			auto camera = frameInfo.MapCamera();
			memcpy(&camera.cb, &cam->get_raw_cb().current, sizeof(camera.cb));
			frameInfo.set(graphics);
		}


	//	shader_data.depth_buffer[0] = g_buffer->depth_tex->texture_2d()->get_static_srv();
	//	shader_data.camera_data =  cam->get_const_buffer();
		graphics.draw(4);
	}
}*/
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

/*
CubeMapEnviromentProcessor::CubeMapEnviromentProcessor()
{
	Render::PipelineStateDesc state_desc;
	Render::RootSignatureDesc root_desc;
	state_desc.root_signature= CubeEnvSignature<SignatureCreator>().create_root();

	state_desc.vertex = Render::vertex_shader::get_resource({"shaders\\cubemap_down.hlsl", "VS", 0, {}});
	state_desc.rtv.rtv_formats.resize(1);
	state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
	char* counts[] = {"1", "8", "32", "64", "128"};

	for (int i = 0; i < 5; i++)
	{
		state_desc.pixel = Render::pixel_shader::get_resource({
			"shaders\\cubemap_down.hlsl", "PS", 0, {D3D::shader_macro("NumSamples", counts[i])}
		});
		states[i].reset(new Render::PipelineState(state_desc));
	}


	state_desc.pixel = Render::pixel_shader::get_resource({"shaders\\cubemap_down.hlsl", "PS_Diffuse", 0, {}});
	state_diffuse.reset(new Render::PipelineState(state_desc));
}

void CubeMapEnviromentProcessor::process(MeshRenderContext::ptr& context, Render::Texture::ptr cubemap,
										 Render::Texture::ptr cubemap_result)
{
	auto& list = context->list->get_graphics();


	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->list->transition(cubemap.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
	context->list->transition(cubemap_result.get(), Render::ResourceState::RENDER_TARGET);


	//MipMapGenerator::get().generate(context->list->get_compute(), cubemap);
	//	for (unsigned int i = 0; i < 6; i++)
	//context->list->transition(cubemap.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, D3D12CalcSubresource(0, i, 0, cubemap->get_desc().MipLevels, cubemap->get_desc().ArraySize()));
	list.set_signature(states[0]->desc.root_signature);
	CubeEnvSignature<Signature> shader_data(&context->list->get_graphics());

	shader_data.source_tex[0] = cubemap->cubemap()->get_srv();

	auto& view = cubemap_result->cubemap();

	for (unsigned int m = 0; m < cubemap_result->get_desc().MipLevels; m++)
	{
		list.set_pipeline(states[std::min(m, 4u)]);
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
}

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