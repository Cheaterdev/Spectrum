#include "pch.h"


SkyRender::SkyRender()
{
	{
		Render::PipelineStateDesc desc;
		{
			Render::RootSignatureDesc root_desc;
			root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 3);
			root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
			root_desc[2] = Render::DescriptorConstants(1, 3, Render::ShaderVisibility::PIXEL);
			root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 3, 1);

			root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
			root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
			desc.root_signature.reset(new Render::RootSignature(root_desc));
		}
		desc.rtv.rtv_formats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.blend.render_target[0].enabled = true;
		desc.blend.render_target[0].source = D3D12_BLEND::D3D12_BLEND_ONE;
		desc.blend.render_target[0].dest = D3D12_BLEND::D3D12_BLEND_ONE;


		desc.pixel = Render::pixel_shader::get_resource({"shaders\\sky.hlsl", "PS", 0, {}});
		desc.vertex = Render::vertex_shader::get_resource({"shaders\\sky.hlsl", "VS", 0, {}});
		state.reset(new Render::PipelineState(desc));
	}
	{
		Render::PipelineStateDesc state_desc;
		{
			Render::RootSignatureDesc root_desc;
			root_desc[0] = Render::DescriptorConstants(0, 1, Render::ShaderVisibility::ALL);
			root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 3);
			root_desc[2] = Render::DescriptorConstants(1, 3, Render::ShaderVisibility::PIXEL);
			root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
			root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
			state_desc.root_signature.reset(new Render::RootSignature(root_desc));
		}
		state_desc.pixel = Render::pixel_shader::get_resource({"shaders\\cubemap.hlsl", "PS", 0, {}});
		state_desc.vertex = Render::vertex_shader::get_resource({"shaders\\cubemap.hlsl", "VS", 0, {}});
		state_desc.rtv.rtv_formats.resize(1);
		state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		cubemap_state.reset(new Render::PipelineState(state_desc));
	}
	transmittance = Render::Texture::get_resource({"textures\\Transmit.dds", false, false});
	irradiance = Render::Texture::get_resource({"textures\\irradianceTexture.dds", false, false});
	inscatter = Render::Texture::get_resource({"textures\\inscatterTexture.dds", false, false});
	table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(3);
	table[0] = transmittance->texture_2d()->srv();
	table[1] = irradiance->texture_2d()->srv();
	table[2] = inscatter->texture_3d()->srv();
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

	//
	//	buffer.size.register_change(this, [this, &buffer](const ivec2& size) {
	//		table[3] = buffer.depth_tex->texture_2d()->srv();
	//	});
}

void SkyRender::process(MeshRenderContext::ptr& context)
{
	auto timer = context->list->start(L"sky");
	auto& list = context->list->get_graphics();

	list.set_pipeline(state);
	list.set_dynamic(0, 0, table);
	list.set_constants(2, context->sky_dir.x, context->sky_dir.y, context->sky_dir.z);
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


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
		list.set_viewport(view->get_viewport());
		list.set_scissor(view->get_scissor());
		list.set_rtv(1, view->get_rtv(), Render::Handle());
		list.set_dynamic(3, 0, g_buffer->depth_tex->texture_2d()->get_static_srv());
		list.set(1, cam->get_const_buffer());
		list.draw(4);
	}
}

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
	auto& list = context->list->get_graphics();
	auto& view = cubemap->cubemap();
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_pipeline(cubemap_state);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());
	list.set_dynamic(1, 0, table);
	list.set_constants(2, context->sky_dir.x, context->sky_dir.y, context->sky_dir.z);
	//	list.assume_state(cubemap, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.transition(cubemap, Render::ResourceState::RENDER_TARGET);

	for (unsigned int i = 0; i < 6; i++)
	{
		list.set_rtv(1, view->get_rtv(i, 0), Render::Handle());
		list.set_constants(0, i);
		list.draw(4);
	}

	MipMapGenerator::get().generate(list.get_compute(), cubemap, cubemap->cubemap());


	CubeMapEnviromentProcessor::get().process(context, cubemap, enviroment.prefiltered_cubemap);
	CubeMapEnviromentProcessor::get().process_diffuse(context, cubemap, enviroment.prefiltered_cubemap_diffuse);
	//    list.transition(cubemap, Render::ResourceState::PIXEL_SHADER_RESOURCE);
}

CubeMapEnviromentProcessor::CubeMapEnviromentProcessor()
{
	Render::PipelineStateDesc state_desc;
	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorConstants(0, 4, Render::ShaderVisibility::ALL);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1);
	//   root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 2);
	root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
	state_desc.root_signature.reset(new Render::RootSignature(root_desc));

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


	//MipMapGenerator::get().generate(context->list->get_compute(), cubemap/*, cubemap->texture_2d()*/);
	//	for (unsigned int i = 0; i < 6; i++)
	//context->list->transition(cubemap.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, D3D12CalcSubresource(0, i, 0, cubemap->get_desc().MipLevels, cubemap->get_desc().ArraySize()));
	list.set_signature(states[0]->desc.root_signature);

	//  context->list->get_graphics().set(2, Render::DescriptorHeapManager::get().get_default_samplers());
	context->list->get_graphics().set_dynamic(1, 0, cubemap->cubemap()->get_srv());

	auto& view = cubemap_result->cubemap();

	for (unsigned int m = 0; m < cubemap_result->get_desc().MipLevels; m++)
	{
		list.set_pipeline(states[std::min(m, 4u)]);
		list.set_viewport(view->get_viewport(m));
		list.set_scissor(view->get_scissor(m));

		for (unsigned int i = 0; i < 6; i++)
		{
			context->list->get_graphics().set_rtv(1, view->get_rtv(i, m), Render::Handle());
			context->list->get_graphics().set_constants(0, i, (float(m) + 0.5f) / cubemap->get_desc().MipLevels,
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

	context->list->get_graphics().set_dynamic(1, 0, cubemap->cubemap()->get_srv());

	auto& view = cubemap_result->cubemap();


	int m = 0;

	list.set_viewport(view->get_viewport(m));
	list.set_scissor(view->get_scissor(m));

	for (unsigned int i = 0; i < 6; i++)
	{
		context->list->get_graphics().set_rtv(1, view->get_rtv(i, m), Render::Handle());
		context->list->get_graphics().set_constants(0, i, (float(m) + 0.5f) / cubemap->get_desc().MipLevels,
		                                            (unsigned int)cubemap->get_desc().Width);
		list.draw(4);
	}
}
