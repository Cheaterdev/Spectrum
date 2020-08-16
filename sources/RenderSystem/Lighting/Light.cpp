#include "pch.h"
#include "Light.h"

#ifdef WTF


template <class T>
struct LightSignature : public T
{
	using T::T;

	/*	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
	root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
	root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
	root_desc[3] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::ALL);
	root_desc[4] = Render::DescriptorSRV(5, Render::ShaderVisibility::ALL);
	root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 3);*/


	typename T::template Table			<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 4>	g_buffer = this;
	typename T::template Table			<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1>	target = this;
	typename T::template ConstBuffer	<2, Render::ShaderVisibility::ALL, 0>									camera_data = this;
	typename T::template ConstBuffer	<3, Render::ShaderVisibility::ALL, 1>									light_count = this;
	typename T::template SRV			<4, Render::ShaderVisibility::ALL, 5>									light_data = this;
	typename T::template Table			<5, Render::ShaderVisibility::ALL, Render::DescriptorRange::SAMPLER, 0, 3>	samplers = this;

};


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
	Render::ComputePipelineStateDesc desc;

	desc.shader = Render::compute_shader::get_resource({ "shaders\\Lighting.hlsl", "CS", 0,{} });
	desc.root_signature = LightSignature<SignatureCreator>().create_root();

	state.reset(new  Render::ComputePipelineState(desc));
}

 void LightSystem::process(MeshRenderContext::ptr & context, G_Buffer & buffer, main_renderer::ptr r, scene_object::ptr scene)
{
	auto timer = context->list->start(L"lighting");
	size_t light_count = lights.size();

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
		LightSignature<Signature> sig(&context->list->get_compute());


		sig.g_buffer[0]= buffer.srv_table;
		sig.target[0] = buffer.result_tex.first()->texture_2d()->get_uav();

		sig.camera_data = context->cam->get_const_buffer();
		sig.light_count.set_raw(light_count); //WTF
		sig.light_data=light_cb; 
		sig.samplers = Render::DescriptorHeapManager::get().get_default_samplers();

	//	context->list->get_compute().set_const_buffer(2, context->cam->get_const_buffer());
		//context->list->get_compute().set_const_buffer(3, light_count);
	//	context->list->get_compute().set_srv(4, light_cb);
		//context->list->get_compute().set(5, Render::DescriptorHeapManager::get().get_default_samplers());
		ivec2 sizes = { buffer.result_tex.first()->get_desc().Width , buffer.result_tex.first()->get_desc().Height };
		context->list->get_compute().dispach(sizes, { 16, 16 });
		context->list->transition_uav(buffer.result_tex.first().get());
	}

	{
		pssm.generate_shadow_map(context, r, scene);

	
		pssm.process(context);
	
	
	}
}


#endif