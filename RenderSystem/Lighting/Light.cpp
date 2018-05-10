#include "pch.h"
#include "Light.h"

 LightSystem::LightSystem()
{
	std::minstd_rand m_gen;
	auto f = [&m_gen](vec3 min, vec3 max)->vec3
	{
		return vec3(std::uniform_real_distribution<float>(min.x, max.x)(m_gen),
			std::uniform_real_distribution<float>(min.y, max.y)(m_gen),
			std::uniform_real_distribution<float>(min.z, max.z)(m_gen));

	};

	for (int i = 0; i < 0; i++)
	{
		Light l;
		l.position = f({ -100, -100, -100 }, { 100, 100, 100 });
		l.color = f({ 1, 1, 1 }, { 4, 4, 4 });
		l.radius = 0.5f;
		lights.push_back(l);
	}

	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
	root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
	root_desc[3] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::ALL);
	root_desc[4] = Render::DescriptorSRV(5, Render::ShaderVisibility::ALL);
	root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 3);
	Render::ComputePipelineStateDesc desc;
	desc.shader = Render::compute_shader::get_resource({ "shaders\\Lighting.hlsl", "CS", 0,{} });
	desc.root_signature.reset(new Render::RootSignature(root_desc));
	state.reset(new  Render::ComputePipelineState(desc));
}

 void LightSystem::process(MeshRenderContext::ptr & context, G_Buffer & buffer, main_renderer::ptr r, scene_object::ptr scene)
{
	auto timer = context->list->start(L"lighting");
	int light_count = lights.size();

	if (light_count)
	{
		std::vector<LightSRV> light_cb(light_count);

		for (unsigned int i = 0; i < lights.size(); i++)
		{
			float sum = lights[i].color.x + lights[i].color.y + lights[i].color.z;
			light_cb[i].position_dist = float4(lights[i].position, lights[i].radius * sum * sum);
			light_cb[i].color_radius = float4(lights[i].color, lights[i].radius);
		}

		context->list->get_compute().set_pipeline(state);
		context->list->get_compute().set_dynamic(0, 0, buffer.srv_table);
		context->list->get_compute().set_table(1, buffer.result_tex.first()->texture_2d()->get_uav());
		context->list->get_compute().set(2, context->cam->get_const_buffer());
		context->list->get_compute().set_const_buffer(3, light_count);
		context->list->get_compute().set_srv(4, light_cb);
		context->list->get_compute().set(5, Render::DescriptorHeapManager::get().get_default_samplers());
		ivec2 sizes = { buffer.result_tex.first()->get_desc().Width , buffer.result_tex.first()->get_desc().Height };
		context->list->get_compute().dispach(sizes, { 16, 16 });
		context->list->transition_uav(buffer.result_tex.first().get());
	}

	{
		pssm.generate_shadow_map(context, r, scene);

	
		pssm.process(context);
	
	
	}
}
