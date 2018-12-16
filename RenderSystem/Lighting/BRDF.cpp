
#include <pch.h>

void BRDF::create_new()
{

	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);

	/*root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
	root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
	root_desc[3] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::ALL);
	root_desc[4] = Render::DescriptorSRV(5, Render::ShaderVisibility::ALL);
	root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 3);*/
	Render::ComputePipelineStateDesc desc;
	desc.shader = Render::compute_shader::get_resource({ "shaders\\BRDF.hlsl", "CS", 0,{} });
	desc.root_signature.reset(new Render::RootSignature(root_desc));
	state.reset(new Render::ComputePipelineState(desc));

	texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 16, 16, 16, 1,  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));


	Render::CommandList::ptr list(new Render::CommandList(Render::CommandListType::DIRECT));
	list->begin("BRDF");


	Render::ComputeContext& compute_context = list->get_compute();
	compute_context.set_pipeline(state);
	compute_context.set_dynamic(0, 0, texture->texture_3d()->get_static_uav());
	compute_context.transition(texture, Render::ResourceState::UNORDERED_ACCESS);
	compute_context.dispach(ivec3(16,16,16), ivec3(4,4,4));
//	compute_context.transition_uav(texture.get());
	compute_context.transition(texture, Render::ResourceState::PIXEL_SHADER_RESOURCE);

	list->end();
	list->execute_and_wait();


}
BOOST_CLASS_EXPORT_IMPLEMENT(BRDF);