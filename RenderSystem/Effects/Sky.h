#pragma once


class CubeMapEnviromentProcessor : public Singleton<CubeMapEnviromentProcessor>
{

	Render::PipelineState::ptr state;

public:

	CubeMapEnviromentProcessor();

	void process(MeshRenderContext::ptr& context, Render::Texture::ptr cubemap);
};



/*
class CubeMapEnviromentProcessor : public Singleton<CubeMapEnviromentProcessor>
{

Render::PipelineState::ptr state;

public:

CubeMapEnviromentProcessor()
{
Render::PipelineStateDesc state_desc;
Render::RootSignatureDesc root_desc;
root_desc[0] = Render::DescriptorConstants(0, 2, Render::ShaderVisibility::ALL);
root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1);
//   root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 2);
root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
state_desc.root_signature.reset(new Render::RootSignature(root_desc));
state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\cubemap_down.hlsl", "PS", 0,{} });
state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\cubemap_down.hlsl", "VS", 0,{} });
state_desc.rtv.rtv_formats.resize(1);
state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
state.reset(new Render::PipelineState(state_desc));
}

void process(MeshRenderContext::ptr& context, Render::Texture::ptr cubemap)
{
auto& list = context->list->get_graphics();
auto& view = cubemap->cubemap();
list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
list.set_pipeline(state);

for (unsigned int i = 0; i < 6; i++)
context->list->transition(cubemap.get(), Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE, D3D12CalcSubresource(0, i, 0, cubemap->get_desc().MipLevels, cubemap->get_desc().ArraySize()));

//  context->list->get_graphics().set(2, Render::DescriptorHeapManager::get().get_default_samplers());

for (unsigned int m = 1; m < cubemap->get_desc().MipLevels; m++)
{
context->list->get_graphics().set_dynamic(1, 0, view->get_srv(m - 1));
list.set_viewport(view->get_viewport(m));
list.set_scissor(view->get_scissor(m));

for (unsigned int i = 0; i < 6; i++)
{
context->list->get_graphics().set_rtv(1, view->get_rtv(i, m), Render::Handle());
context->list->get_graphics().set_constants(0, i, m);
list.draw(4);
context->list->transition(cubemap.get(), Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE, D3D12CalcSubresource(m, i, 0, cubemap->get_desc().MipLevels, cubemap->get_desc().ArraySize()));
}
}
}
};
*/


class SkyRender :public Events::prop_handler
{
	Render::PipelineState::ptr state;
	Render::PipelineState::ptr cubemap_state;

	Render::Texture::ptr transmittance;
	Render::Texture::ptr irradiance;
	Render::Texture::ptr inscatter;
	Render::HandleTable table;
//	G_Buffer& buffer;
	bool processed = false;
public:
	using ptr = std::shared_ptr<SkyRender>;
	Render::Texture::ptr cubemap;

	float3 dir = { 0,0,1 };


	SkyRender();


	void process(MeshRenderContext::ptr& context);


	void update_cubemap(MeshRenderContext::ptr& context);
};
