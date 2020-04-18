#include "pch.h"

template <class T>
struct GBufferDownsamplerSignature : public T
{
	using T::T;

	/*		root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 8);
			root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 8, 1);
			root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::PIXEL);
			root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::PIXEL, 0, 2);
			root_desc[4] = Render::DescriptorConstants(1, 3, Render::ShaderVisibility::PIXEL);

					root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
				root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
			
			*/
	typename T::template Table			<0, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 2>	g_buffer = this;
//	typename T::template Table			<1, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 8>	unknown = this;
	typename T::template ConstBuffer	<1, Render::ShaderVisibility::PIXEL, 0>									camera_data = this;
	//typename T::template Table			<3, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SAMPLER, 0, 2>	samplers = this;
//	typename T::template Constants		<4, Render::ShaderVisibility::ALL, 1, 3>								constants = this;

	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearWrapDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };


};


class GBufferDownsampler :public Events::prop_handler
{
	Render::PipelineState::ptr downsample_state;
	Render::PipelineState::ptr copy_state;
	Render::PipelineState::ptr state;
	Render::PipelineState::ptr last_state;

	Render::HandleTable ao_mips;
	Render::HandleTable depth_mips;
public:
	using ptr = std::shared_ptr<GBufferDownsampler>;
	Render::Texture::ptr ao_texture;
	G_Buffer& g_buffer;
	std::vector<Render::HandleTable> downsample_srv;
	std::vector<Render::HandleTable> downsample_rtv;

	std::vector<Render::HandleTable> downsample_srv_pow2;
	std::vector<Render::HandleTable> downsample_rtv_pow2;

	//    Render::HandleTable last_rtv;
	Render::HandleTable default_srv;




	GBufferDownsampler(G_Buffer& buffer) :g_buffer(buffer)
	{




		/*Render::PipelineStateDesc desc;
	
		desc.root_signature = GBufferDownsamplerSignature<SignatureCreator>().create_root();
		desc.rtv.rtv_formats = { DXGI_FORMAT_R8_UNORM };
		desc.blend.render_target[0].enabled = false;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\Ambient.hlsl", "PS", 0,{} });
		desc.vertex = Render::vertex_shader::get_resource({ "shaders\\Ambient.hlsl", "VS", 0,{} });
		state.reset(new  Render::PipelineState(desc));
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\Ambient.hlsl", "PS_LAST", 0,{} });
		desc.rtv.rtv_formats = { DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT };
		desc.blend.render_target[0].enabled = false;
		desc.blend.render_target[1].enabled = true;
		desc.blend.render_target[1].source = D3D12_BLEND::D3D12_BLEND_ONE;
		desc.blend.render_target[1].dest = D3D12_BLEND::D3D12_BLEND_ONE;
		desc.blend.independ_blend = true;
		last_state.reset(new  Render::PipelineState(desc));*/
		ao_mips = Render::DescriptorHeapManager::get().get_csu_static()->create_table(3);
		depth_mips = Render::DescriptorHeapManager::get().get_csu_static()->create_table(3);
		{
			Render::PipelineStateDesc desc;
			{
				Render::RootSignatureDesc root_desc;
				root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 2);
				//  root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 2);
				root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
				root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearWrapDesc);
				root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerPointClampDesc);
				desc.root_signature.reset(new Render::RootSignature(root_desc));
			}
			desc.blend.render_target[0].enabled = false;
			desc.rtv.rtv_formats = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM };
			desc.pixel = Render::pixel_shader::get_resource({ "shaders\\Downsample.hlsl", "PS", 0,{} });
			desc.vertex = Render::vertex_shader::get_resource({ "shaders\\Downsample.hlsl", "VS", 0,{} });
			downsample_state.reset(new  Render::PipelineState(desc));

			desc.pixel = Render::pixel_shader::get_resource({ "shaders\\Downsample.hlsl", "PS_Copy", 0,{} });

			copy_state.reset(new  Render::PipelineState(desc));
		}
		//            last_rtv = Render::DescriptorHeapManager::get().get_rt()->create_table(2);
		default_srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(2 + 3 + 3 + 3);

		buffer.size.register_change(this, [this](const ivec2& size) {


			auto& depth_tex_mips = g_buffer.depth_tex_mips;
			auto& depth_tex_mips_pow2 = g_buffer.depth_tex_mips_pow2;
			ao_texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8_UNORM, size.x, size.y, 1, 3, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
			ao_mips[0] = ao_texture->texture_2d()->srv(0);
			ao_mips[1] = ao_texture->texture_2d()->srv(1);
			ao_mips[2] = ao_texture->texture_2d()->srv(2);
			depth_mips[0] = depth_tex_mips->texture_2d()->srv(0);
			depth_mips[1] = depth_tex_mips->texture_2d()->srv(1);
			depth_mips[2] = depth_tex_mips->texture_2d()->srv(2);
			downsample_srv.resize(depth_tex_mips->get_desc().MipLevels);
			downsample_rtv.resize(depth_tex_mips->get_desc().MipLevels);

			for (auto i = 0; i < depth_tex_mips->get_desc().MipLevels; i++)
			{
				downsample_srv[i] = Render::DescriptorHeapManager::get().get_csu_static()->create_table(2);
				downsample_rtv[i] = Render::DescriptorHeapManager::get().get_rt()->create_table(2);
				downsample_rtv[i][0] = depth_tex_mips->texture_2d()->rtv(i);
				downsample_rtv[i][1] = g_buffer.normal_tex->texture_2d()->rtv(i);
				downsample_srv[i][0] = depth_tex_mips->texture_2d()->srv(i);
				downsample_srv[i][1] = g_buffer.normal_tex->texture_2d()->srv(i);
			}


			downsample_srv_pow2.resize(depth_tex_mips_pow2->get_desc().MipLevels);
			downsample_rtv_pow2.resize(depth_tex_mips_pow2->get_desc().MipLevels);

			for (auto i = 0; i < depth_tex_mips_pow2->get_desc().MipLevels; i++)
			{
				downsample_srv_pow2[i] = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
				downsample_rtv_pow2[i] = Render::DescriptorHeapManager::get().get_rt()->create_table(1);
				downsample_rtv_pow2[i][0] = depth_tex_mips_pow2->texture_2d()->rtv(i);
				downsample_srv_pow2[i][0] = depth_tex_mips_pow2->texture_2d()->srv(i);
			}



			//     last_rtv[0] = ao_texture->texture_2d()->rtv(0);
			/* last_rtv[1] = g_buffer->light_tex->texture_2d()->rtv(0);*/
			default_srv[0] = g_buffer.albedo_tex->texture_2d()->srv();
			default_srv[1] = g_buffer.normal_tex->texture_2d()->srv();

			for (auto i = 0; i < 3; i++)
				default_srv[2 + i] = depth_tex_mips->texture_2d()->srv(i);

			for (auto i = 0; i < 3; i++)
				default_srv[5 + i] = ao_texture->texture_2d()->srv(i);
			});
	}



	void process(MeshRenderContext::ptr& context/*, Render::CubemapView::ptr& cube*/)
	{
		auto timer = context->list->start(L"ambient");

		auto& list = *context->list;
		auto& graphics = context->list->get_graphics();
		auto& view = ao_texture->texture_2d();
		auto& depth_tex_mips = g_buffer.depth_tex_mips;
		//	list.assume_state(ao_texture, Render::ResourceState::PIXEL_SHADER_RESOURCE);
		//	auto normal_state = g_buffer->normal_tex->get_state(list.get_id());
		//	auto depth_state = depth_tex_mips->get_state(list.get_id());
		list.transition(ao_texture.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);

		list.transition(depth_tex_mips.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, 0);
		list.transition(g_buffer.normal_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, 0);

		graphics.set_topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


		GBufferDownsamplerSignature<Signature> shader_data(&graphics);
		{
			auto& depth_tex_mips_pow2 = g_buffer.depth_tex_mips_pow2;
			auto& depth_view_pow2 = depth_tex_mips_pow2->texture_2d();

			graphics.set_pipeline(copy_state);

			list.transition(depth_tex_mips_pow2.get(), Render::ResourceState::RENDER_TARGET, 0);
			graphics.set_viewport(depth_view_pow2->get_viewport(0));
			graphics.set_scissor(depth_view_pow2->get_scissor(0));
			graphics.set_rtv(downsample_rtv_pow2[0], Render::Handle());
			shader_data.g_buffer[0]= downsample_srv[0][0];
			//	list.set_dynamic(0, 1, downsample_srv[i - 1][1]);
			graphics.draw(4);
			list.transition(depth_tex_mips_pow2.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, 0);
			//		list.transition(g_buffer.normal_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, i)



			graphics.set_pipeline(downsample_state);



			for (int i = 1; i < depth_tex_mips_pow2->get_desc().MipLevels; i++)
			{
				list.transition(depth_tex_mips_pow2.get(), Render::ResourceState::RENDER_TARGET, i);
				graphics.set_viewport(depth_view_pow2->get_viewport(i));
				graphics.set_scissor(depth_view_pow2->get_scissor(i));
				graphics.set_rtv(downsample_rtv_pow2[i], Render::Handle());
				shader_data.g_buffer[0] = downsample_srv_pow2[i - 1][0];
				//	list.set_dynamic(0, 1, downsample_srv[i - 1][1]);
				graphics.draw(4);
				list.transition(depth_tex_mips_pow2.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
				//		list.transition(g_buffer.normal_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
			}
		}


		//     list.set(1, Render::DescriptorHeapManager::get().get_default_samplers());
		shader_data.camera_data= context->cam->get_const_buffer();

		auto& depth_view = depth_tex_mips->texture_2d();

		for (int i = 1; i < depth_tex_mips->get_desc().MipLevels; i++)
		{
			list.transition(g_buffer.normal_tex.get(), Render::ResourceState::RENDER_TARGET, i);
			list.transition(depth_tex_mips.get(), Render::ResourceState::RENDER_TARGET, i);
			graphics.set_viewport(depth_view->get_viewport(i));
			graphics.set_scissor(depth_view->get_scissor(i));
			graphics.set_rtv(downsample_rtv[i], Render::Handle());
			shader_data.g_buffer[0] = downsample_srv[i - 1][0];
			shader_data.g_buffer[1] = downsample_srv[i - 1][1];
			graphics.draw(4);
			list.transition(depth_tex_mips.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
			list.transition(g_buffer.normal_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
		}







		//depth_tex_mips->assume_state(Render::ResourceState::PIXEL_SHADER_RESOURCE);
		//	g_buffer->normal_tex->assume_state(Render::ResourceState::PIXEL_SHADER_RESOURCE);


		/*
		list.transition(ao_texture.get(), Render::ResourceState::RENDER_TARGET);
		list.set_pipeline(state);
		list.set(0, default_srv);
		list.set_const_buffer(2, context->cam->get_const_buffer());
		list.set(3, Render::DescriptorHeapManager::get().get_default_samplers());

		for (auto i = ao_texture->get_desc().MipLevels - 1; i > 0; i--)
		{
		list.set_viewport(view->get_viewport(i));
		list.set_scissor(view->get_scissor(i));
		list.set_rtv(1, view->get_rtv(i), Render::Handle());
		list.set_constants(4, i, view->get_scissor(i).right,  view->get_scissor(i).bottom);
		list.draw(4);
		list.transition(ao_texture.get(), Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE, i);
		}

		list.transition(g_buffer->result_tex.first(), Render::ResourceState::RENDER_TARGET);
		list.set_pipeline(last_state);
		list.set(1, cube->get_srv());
		list.set_viewport(view->get_viewport(0));
		list.set_scissor(view->get_scissor(0));
		list.set_rtvs(Render::Handle(), ao_texture->texture_2d()->get_rtv(), g_buffer->result_tex.first()->texture_2d()->get_rtv());
		list.draw(4);
		list.transition(ao_texture.get(), Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE, 0);*/
	}


};


template <class T>
struct EffectSignature : public T
{
	using T::T;
	typename T::template Table<0, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 8> gi_table = this;
	typename T::template Constants<1, Render::ShaderVisibility::PIXEL, 1, 7> constants = this;
	typename T::template Table<2, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 2, 1> voxel_lighted = this;
	typename T::template ConstBuffer<3, Render::ShaderVisibility::PIXEL, 0> camera_data = this;
	typename T::template Table<4, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 6, 2> help_table2 = this;

	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearWrapDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };
	typename T::template Sampler<2, Render::ShaderVisibility::ALL, 0> aniso{ Render::Samplers::SamplerLinearClampDesc, this };

	/*
	auto get_samplers()
	{
		return std::make_tuple(
			Descriptors::Sampler<0, Render::ShaderVisibility::PIXEL, 0>(Render::Samplers::SamplerLinearWrapDesc),
			Descriptors::Sampler<1, Render::ShaderVisibility::PIXEL, 1>(Render::Samplers::SamplerPointClampDesc),
			Descriptors::Sampler<2, Render::ShaderVisibility::PIXEL, 2>(Render::Samplers::SamplerLinearClampDesc));
	}*/


};


template <class T>
struct VoxelMipMapSignature : public T
{
	using T::T;
	typename T::template Table<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 1> source = this;
	typename T::template Table<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 3> dest_mips = this;
	typename T::template Table<2, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 1, 1> visibility = this;
	typename T::template ConstBuffer<3, Render::ShaderVisibility::ALL, 0> voxels_per_tile = this;



	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearBorderDesc, this };

	/*

	auto get_samplers()
	{
		return std::make_tuple(Descriptors::Sampler<0, Render::ShaderVisibility::ALL, 0>(Render::Samplers::SamplerLinearBorderDesc));
	}*/



};


template <class T>
struct VoxelLightingSignature : public T
{
	using T::T;
	/*
			root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 2);
			root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
			root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 2, 1);
			root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 4, 1);
			root_desc[4] = Render::DescriptorSRV(5, Render::ShaderVisibility::ALL);
			root_desc[5] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 6, 1);
			root_desc[6] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 7, 1);
			root_desc[7] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);

			root_desc.set_sampler(0, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerLinearBorderDesc);
			root_desc.set_sampler(1, 0, Render::ShaderVisibility::PIXEL, Render::Samplers::SamplerShadowComparisonDesc);
			*/

	typename T::template Table			<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 2>	srv_table = this;
	typename T::template Table			<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1>	volume_lighted = this;
	typename T::template Table			<2, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 2, 1>	second_bunce_table = this;
	typename T::template Table			<3, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 4, 1>	wtf3 = this;

	typename T::template SRV			<4, Render::ShaderVisibility::ALL, 5>								wtf4 = this;
	typename T::template Table			<5, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 6,1>	sky = this;
	typename T::template Table			<6, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 7,1>	gpu_tiles_buffer = this;
	typename T::template ConstBuffer	<7, Render::ShaderVisibility::ALL, 0>									voxel_info = this;


	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearBorderDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerShadowComparisonDesc, this };

};



void VoxelGI::init_states()
{
	Render::PipelineStateDesc desc;
	desc.root_signature = EffectSignature< SignatureCreator>().create_root();

	desc.blend.render_target[0].dest = D3D12_BLEND_ONE;
	desc.blend.render_target[0].source = D3D12_BLEND_ONE;

	desc.rtv.stencil_desc.StencilFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
	desc.vertex = Render::vertex_shader::get_resource({ "shaders\\voxel_screen.hlsl", "VS", 0,{} });


	for (int i = 0; i < screen_states.size(); i++)
	{

		std::vector<D3D::shader_macro> macros;

		if (i == static_cast<int>(VISUALIZE_TYPE::REFLECTION))
			macros.emplace_back("REFLECTION", "1");

		if (i == static_cast<int>(VISUALIZE_TYPE::INDIRECT))
			macros.emplace_back("INDIRECT", "1");

		if (i == static_cast<int>(VISUALIZE_TYPE::VOXEL))
			macros.emplace_back("SCREEN", "1");

		desc.rtv.enable_stencil = true;
		desc.blend.render_target[0].enabled = false;
		desc.blend.render_target[1].enabled = false;
		desc.rtv.stencil_read_mask = 1;
		desc.rtv.ds_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen.hlsl", "PS", 0,macros });
		desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };
		screen_states[i][1] = std::make_shared<Render::PipelineState>(desc);


		desc.rtv.enable_stencil = false;
		desc.blend.render_target[0].enabled = false;
		desc.rtv.ds_format = DXGI_FORMAT_UNKNOWN;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen.hlsl", "PS_Low", 0,macros });
		desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };
		screen_states[i][0] = std::make_shared<Render::PipelineState>(desc);


		desc.rtv.ds_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen.hlsl", "PS_Resize", 0,macros });
		desc.rtv.enable_stencil = true;
		desc.rtv.stencil_read_mask = 1;
		desc.blend.render_target[0].enabled = false;
		desc.blend.render_target[1].enabled = false;
		desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };
		screen_states[i][2] = std::make_shared<Render::PipelineState>(desc);



		desc.rtv.ds_format = DXGI_FORMAT_UNKNOWN;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen_blur.hlsl", "PS", 0,macros });
		desc.rtv.enable_stencil = true;
		desc.blend.render_target[0].enabled = false;
		desc.blend.render_target[1].enabled = false;
		desc.rtv.rtv_formats = {/*DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,*/ DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };
		screen_states[i][3] = std::make_shared<Render::PipelineState>(desc);


		desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
		desc.rtv.stencil_read_mask = 2;
		desc.rtv.enable_stencil = true;
		desc.rtv.ds_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen_reflection.hlsl", "PS", 0,macros });
		desc.blend.render_target[0].enabled = false;
		//desc.rtv.enable_stencil = false;

		reflection_states[i].reflection_state = std::make_shared<Render::PipelineState>(desc);


		desc.rtv.ds_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen_reflection.hlsl", "PS_resize", 0,macros });
		reflection_states[i].reflection_resize_state = std::make_shared<Render::PipelineState>(desc);
	}


	desc.rtv.ds_format = DXGI_FORMAT_UNKNOWN;
	desc.pixel = Render::pixel_shader::get_resource({ "shaders\\voxel_screen_reflection.hlsl", "PS_low", 0,{} });
	desc.rtv.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };


	downsampled_reflection_state.reset(new  Render::PipelineState(desc));



	{
		Render::ComputePipelineStateDesc desc;
		desc.root_signature = VoxelMipMapSignature< SignatureCreator>().create_root();

		desc.shader = Render::compute_shader::get_resource({ "shaders\\voxel_mipmap.hlsl", "CS", 0,{ D3D::shader_macro("COUNT","3") } });
		downsample_state[2] = std::make_shared<Render::ComputePipelineState>(desc);

		desc.shader = Render::compute_shader::get_resource({ "shaders\\voxel_mipmap.hlsl", "CS", 0,{ D3D::shader_macro("COUNT","2") } });
		downsample_state[1] = std::make_shared<Render::ComputePipelineState>(desc);

		desc.shader = Render::compute_shader::get_resource({ "shaders\\voxel_mipmap.hlsl", "CS", 0,{ D3D::shader_macro("COUNT","1") } });
		downsample_state[0] = std::make_shared<Render::ComputePipelineState>(desc);

	}

	{
		Render::ComputePipelineStateDesc desc;	
		desc.root_signature = VoxelLightingSignature<SignatureCreator>().create_root();

		desc.shader = Render::compute_shader::get_resource({ "shaders\\voxel_lighting.hlsl", "CS", 0,{} });
		lighting_state.reset(new Render::ComputePipelineState(desc));


		desc.shader = Render::compute_shader::get_resource({ "shaders\\voxel_lighting.hlsl", "CS", 0,{ D3D::shader_macro("SECOND_BOUNCE","1") } });
		lighting_state_second_bounce.reset(new Render::ComputePipelineState(desc));
	}

	second_bunce_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);
	second_bunce_table[0] = volume_lighted->texture_3d()->srv(1, 5);
}

void VoxelGI::start_new()
{
	//Log::get() << 1 << Log::endl;

	all_scene_regen_counter = 2;
	tiled_volume_lighted->remove_all();

	//	Log::get() << 2 << Log::endl;

	tiled_volume_normal_static->remove_all();
	tiled_volume_albedo_static->remove_all();
	//Log::get() <<3 << Log::endl;

	tiled_volume_normal->remove_all();
	tiled_volume_albedo->remove_all();
	//	Log::get() << 4 << Log::endl;

	dynamic_generator.remove_all();
	dynamic_generator_voxelizing.remove_all();
	//	Log::get() << 5 << Log::endl;

	for (auto&& b : gpu_tiles_buffer)
		if (b)b->clear();
	tiled_volume_lighted->load_all();
	//	Log::get() << 6 << Log::endl;


	//	tiled_volume_albedo->test();
}

void VoxelGI::resize(ivec2 size)
{
	/*
	//downsampled_light.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, size.x/2, size.y/2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	gi_textures.resize(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, { size.x, size.y });
	downsampled_light.resize(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, { size.x/2, size.y/2 });
	current_gi_texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	downsampled_reflection.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, size.x/2, size.y/2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
	*/

}
VoxelGI::VoxelGI(Scene::ptr& scene, G_Buffer& buffer) :scene(scene), buffer(buffer), reflection_effect(buffer)
{
	scene->on_element_add.register_handler(this, [this](scene_object* object) {
		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			need_start_new = true;
		});


	{
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM, 512, 512, 512, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);
		auto info = Device::get().get_native_device()->GetResourceAllocationInfo(0, 1, &desc);



		tiled_volume_albedo.reset(new Texture3DTiledDynamic(desc));
		tiled_volume_normal.reset(new Texture3DTiledDynamic(desc));

		tiled_volume_albedo_static.reset(new Texture3DTiledDynamic(desc));
		tiled_volume_normal_static.reset(new Texture3DTiledDynamic(desc));

		desc.MipLevels = 9;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tiled_volume_lighted.reset(new Texture3DTiledDynamic(desc));

		lighed_to_albedo_coeff = tiled_volume_lighted->get_tiles_count() / tiled_volume_albedo->get_tiles_count();
		dynamic_generator = TileDynamicGenerator(tiled_volume_lighted->get_tiles_count());
		dynamic_generator_voxelizing = TileDynamicGenerator(tiled_volume_albedo->get_tiles_count());
		visibility = tiled_volume_lighted->create_visibility();

		tiled_volume_lighted->on_tile_load = [this](Tile::ptr& tile)
		{

			if (gpu_tiles_buffer[tile->mip_level])
			{
				gpu_tiles_buffer[tile->mip_level]->insert(tile->position);
			}

			tiled_volume_lighted->create_tile(tile->position / 2, tile->mip_level + 1);
		};
		tiled_volume_lighted->on_tile_remove = [this](Tile::ptr& tile)
		{

			if (gpu_tiles_buffer[tile->mip_level])
			{
				gpu_tiles_buffer[tile->mip_level]->erase(tile->position);
			}

			//	tiled_volume_lighted->create_tile(tile->position / 2, tile->mip_level + 1);
		};

		visibility->on_process = [this](ivec3 pos, int mip) {

			if (mip == 0)
			{

				auto& tile1 = tiled_volume_albedo->create_static_tile(pos / lighed_to_albedo_coeff, mip);
				auto& tile2 = tiled_volume_normal->create_static_tile(pos / lighed_to_albedo_coeff, mip);
				auto& tile3 = tiled_volume_lighted->create_static_tile(pos, mip);
				//	tile->last_visible = std::chrono::system_clock::now().time_since_epoch().count();
			}
			else
			{
				/*	auto &tile = tiled_volume_lighted->get_tile(0,pos);
				if (tile&&	tile->last_visible <std::chrono::system_clock::now().time_since_epoch().count()-5000)
				{
				remove_static_tiles(tile->position);
				}
				*/
			}
		};


		//	tiled_volume_lighted->load_all();


		volume_albedo = tiled_volume_albedo->texture;
		volume_normal = tiled_volume_normal->texture;
		volume_lighted = tiled_volume_lighted->texture;

		uav_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(3);
		srv_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(2);

		uav_table[0] = volume_albedo->texture_3d()->uav();
		uav_table[1] = volume_normal->texture_3d()->uav();
		uav_table[2] = visibility->uav();

		srv_table[0] = volume_albedo->texture_3d()->srv();
		srv_table[1] = volume_normal->texture_3d()->srv();

		gpu_tiles_buffer.resize(desc.MipLevels);
		gpu_tiles_buffer[0].reset(new GPUTilesBuffer);
		gpu_tiles_buffer[1].reset(new GPUTilesBuffer);
		gpu_tiles_buffer[4].reset(new GPUTilesBuffer);
		gpu_tiles_buffer[7].reset(new GPUTilesBuffer);

		gpu_tiles_buffer[0]->set_size(tiled_volume_lighted->get_max_tiles_count());
		gpu_tiles_buffer[1]->set_size(tiled_volume_lighted->get_max_tiles_count(1));
		gpu_tiles_buffer[4]->set_size(tiled_volume_lighted->get_max_tiles_count(1));
		gpu_tiles_buffer[7]->set_size(tiled_volume_lighted->get_max_tiles_count(1));


		gi_rtv = Render::DescriptorHeapManager::get().get_rt()->create_table(2);

	}
	init_states();

	//	buffer.size.register_change(this, [this](ivec2 size) {resize(size); });
}

void VoxelGI::voxelize(MeshRenderContext::ptr& context, main_renderer::ptr r)
{
	auto& graphics = context->list->get_graphics();
	auto& list = *context->list;


	static bool prev = 0;
	bool cur = !!GetAsyncKeyState('P');

	if (!cur && prev)
	{
		start_new();
		//all_scene_regen_counter++;
	}

	if (all_scene_regen_counter > 0)
	{
		visibility->wait_for_results();
	}
	prev = cur;
	if (!all_scene_regen_counter) {
		dynamic_generator.begin(context->voxel_min, context->voxel_min + context->voxel_size);
		dynamic_generator_voxelizing.begin(context->voxel_min, context->voxel_min + context->voxel_size);

		r->iterate(MESH_TYPE::DYNAMIC, [this](scene_object::ptr& obj) {
			auto min = obj->get_min();
			auto max = obj->get_max();

			dynamic_generator.add(min, max);
			dynamic_generator_voxelizing.add(min, max);
			}
		);
		dynamic_generator.end();
		dynamic_generator_voxelizing.end();
	}

	for (auto& e : dynamic_generator.tiles_to_load)
	{

		auto pos = dynamic_generator.get_pos(e);

		auto& tile = tiled_volume_lighted->get_tile(0, pos);

		if (!tile || !tile->has_static)
		{
			auto& tile2 = tiled_volume_lighted->create_tile(pos, 0);
			tile2->has_dynamic = true;
		}

	}

	for (auto& e : dynamic_generator_voxelizing.tiles_to_load)
	{
		auto pos = dynamic_generator_voxelizing.get_pos(e);
		auto& tile1 = tiled_volume_albedo->create_dynamic_tile(pos, 0);
		auto& tile2 = tiled_volume_normal->create_dynamic_tile(pos, 0);
	}


	for (auto& e : dynamic_generator.tiles_to_remove)
	{
		auto pos = dynamic_generator.get_pos(e);

		auto& tile = tiled_volume_lighted->get_tile(0, pos);

		if (!tile)
			continue;
		if (!tile->has_static)
		{
			tile->has_static = false;
			tile->has_dynamic = false;
			tiled_volume_lighted->remove_tile(tile);



		}
		else
		{
			tile->has_dynamic = false;
		}

		//	remove_dynamic_tiles(pos);

	}

	for (auto& e : dynamic_generator_voxelizing.tiles_to_remove)
	{
		auto pos = dynamic_generator_voxelizing.get_pos(e);
		tiled_volume_albedo->remove_dynamic_tile(pos, 0);
		tiled_volume_normal->remove_dynamic_tile(pos, 0);
	}


	auto timer = context->list->start(L"voxelizing");

	list.transition(volume_albedo, Render::ResourceState::UNORDERED_ACCESS);
	list.transition(volume_normal, Render::ResourceState::UNORDERED_ACCESS);
	float values[4] = { 0, 0, 0, 0 };
	list.flush_transitions();

	tiled_volume_albedo->clear_dynamic(graphics.get_base());
	if (clear_scene && all_scene_regen_counter)
	{
		auto timer = context->list->start(L"clear");
		//if(all_scene_regen_counter)
		//tiled_volume_albedo->clear_static(graphics);
	//	dynamic_generator_voxelizing.remove_all();

	}
	else
	{
		for (auto& e : dynamic_generator_voxelizing.dynamic_tiles)
		{
			auto pos = dynamic_generator_voxelizing.get_pos(e);

			auto albedo_tile = tiled_volume_albedo->get_tile(0, pos);
			auto normal_tile = tiled_volume_normal->get_tile(0, pos);
			if (albedo_tile->page_backup)
			{
				auto from = albedo_tile->page_backup->get_tile_texture(albedo_tile->tile_offset_backup);
				auto to = albedo_tile->page->get_tile_texture(albedo_tile->tile_offset);

				context->list->get_copy().copy_texture(to, 0, from, 0);
			}


			if (normal_tile->page_backup)
			{
				auto from = normal_tile->page_backup->get_tile_texture(normal_tile->tile_offset_backup);
				auto to = normal_tile->page->get_tile_texture(normal_tile->tile_offset);

				context->list->get_copy().copy_texture(to, 0, from, 0);
			}
		}


	}


	context->render_type = RENDER_TYPE::VOXEL;
	context->render_mesh = MESH_TYPE::DYNAMIC;
	if (all_scene_regen_counter)
	{
		context->render_mesh = MESH_TYPE::STATIC;
	}
	context->voxel_target = uav_table;
	context->voxel_target_size = volume_lighted->get_size();

	context->pipeline.rtv.rtv_formats.clear();
	context->pipeline.rtv.enable_depth = false;
	context->pipeline.rtv.enable_depth_write = false;
	context->pipeline.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

	graphics.set_viewport({ 0, 0, volume_albedo->get_size().xy });
	graphics.set_scissor({ 0, 0, volume_albedo->get_size().xy });
	graphics.set_rtv(0, Render::Handle(), Render::Handle());

	{
		auto timer = context->list->start(L"tiled_update");
		tiled_volume_albedo->update();
		tiled_volume_normal->update();
		tiled_volume_lighted->update();
	}



	{
		auto timer = context->list->start(L"render");
		r->render(context, scene);
	}


	if (all_scene_regen_counter)
	{
		auto timer = context->list->start(L"visibility update");
		visibility->update(context->list);

	}

	for (auto&& b : gpu_tiles_buffer)
		if (b)
			b->update(context->list);

	if (all_scene_regen_counter > 0)
		all_scene_regen_counter--;
}

void VoxelGI::generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm, Enviroment enviroment)
{
	auto timer = context->list->start(L"GI");
	{

		if (need_start_new)
		{
			start_new();
			need_start_new = false;
		}

		context->voxel_min = scene->get_min() - float3(1, 1, 1);
		context->voxel_size = scene->get_max() + float3(1, 1, 1) - context->voxel_min;
		context->voxel_size.x = context->voxel_size.y = context->voxel_size.z = max(200.0f, context->voxel_size.max_element());

		context->voxel_tiles_count = tiled_volume_lighted->get_tiles_count(0);
		context->voxels_per_tile = tiled_volume_lighted->get_voxels_per_tile();


		if (voxelize_scene)
			voxelize(context, r);

		light_counter = (light_counter + 1) % 5;

		if (light_scene && light_counter == 0 || all_scene_regen_counter > 0)
		{
			lighting(context, pssm, enviroment.prefiltered_cubemap_diffuse->array_cubemap());
			mipmapping(context);
		}

		if (!downsampler) downsampler = std::make_shared<GBufferDownsampler>(buffer);
		downsampler->process(context);


		screen(context, scene, enviroment.prefiltered_cubemap_diffuse->array_cubemap());
		screen_reflection(context, scene, enviroment.prefiltered_cubemap->array_cubemap());
	}
}

void VoxelGI::lighting(MeshRenderContext::ptr& context, PSSM& pssm, Render::CubemapArrayView::ptr skymap)
{
	auto timer = context->list->start(L"lighting");
	auto& list = *context->list;

	auto& compute = context->list->get_compute();
	if (GetAsyncKeyState('G'))
		compute.set_pipeline(lighting_state);
	else
		compute.set_pipeline(lighting_state_second_bounce);


	VoxelLightingSignature<Signature> shader_data(&compute);
	list.transition(volume_lighted, Render::ResourceState::UNORDERED_ACCESS);
	list.transition(volume_albedo, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.transition(volume_normal, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

	VoxelMipMapSignature<Signature> mip_sig(&compute);

	shader_data.srv_table[0]=srv_table;
	shader_data.volume_lighted[0] = volume_lighted->texture_3d()->get_uav();
	shader_data.second_bunce_table[0] = second_bunce_table;

	pssm.set(context);
	shader_data.sky[0]= skymap->get_srv();
	shader_data.gpu_tiles_buffer[0] = gpu_tiles_buffer[0]->get_srv();


	struct
	{
		vec4 dir;
		vec4 voxel_min;
		vec4 voxel_size;
		ivec3 voxels_per_tile; int groups;
	}voxel_info;

	voxel_info.dir = vec4(context->sky_dir);
	voxel_info.voxel_min = vec4(context->voxel_min, 0);
	voxel_info.voxel_size = vec4(context->voxel_size, 0);
	voxel_info.voxels_per_tile = context->voxels_per_tile;// ivec4(, 0);
	voxel_info.groups = tiled_volume_lighted->get_voxels_per_tile().x * tiled_volume_lighted->get_voxels_per_tile().y * tiled_volume_lighted->get_voxels_per_tile().z / (4 * 4 * 4);


	shader_data.voxel_info.set_raw(voxel_info);

	compute.dispach(ivec3(gpu_tiles_buffer[0]->size() * voxel_info.groups, 1, 1), ivec3(1, 1, 1));
}

void VoxelGI::mipmapping(MeshRenderContext::ptr& context)
{
	auto timer = context->list->start(L"mipmapping");

	auto& compute = context->list->get_compute();
	auto& list = *context->list;

	//auto& mip_sig = MipMapSignature::signature;
	//auto&& mip_sig = compute.wrap<MipMapSignature>();
	VoxelMipMapSignature<Signature> mip_sig(&compute);

	unsigned int mip_count = 0;

	list.transition(volume_lighted.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, mip_count);
	mip_count++;


	struct
	{
		ivec3 voxels_per_tile; int groups;
	}voxel_info;

	voxel_info.voxels_per_tile = context->voxels_per_tile;
	voxel_info.groups = tiled_volume_lighted->get_voxels_per_tile().x * tiled_volume_lighted->get_voxels_per_tile().y * tiled_volume_lighted->get_voxels_per_tile().z / (4 * 4 * 4);

	compute.set_signature(downsample_state[2]->desc.root_signature);
	mip_sig.voxels_per_tile.set_raw(voxel_info);

	//	
	while (mip_count < volume_lighted->get_desc().MipLevels)
	{
		unsigned int current_mips = std::min(3u, volume_lighted->get_desc().MipLevels - mip_count);
		compute.set_pipeline(downsample_state[current_mips - 1]);
		mip_sig.source[0] = volume_lighted->texture_3d()->get_srv(mip_count - 1);

		for (unsigned int i = 0; i < current_mips; i++)
		{
			mip_sig.dest_mips[i] = volume_lighted->texture_3d()->get_uav(mip_count + i);
		}
		mip_sig.visibility[0] = gpu_tiles_buffer[mip_count]->get_srv();

		//if(mip_count==1)
		compute.dispach(ivec3(gpu_tiles_buffer[mip_count]->size() * voxel_info.groups, 1, 1), ivec3(1, 1, 1));

		//compute.dispach(volume_lighted->get_size(mip_count));

		for (unsigned int i = 0; i < current_mips; i++)
		{
			list.transition(volume_lighted.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, mip_count + i);
		}

		mip_count += current_mips;
	}

	//	compute.assume_state(volume_lighted.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
}

void VoxelGI::screen(MeshRenderContext::ptr& context, scene_object::ptr scene, Render::CubemapArrayView::ptr skymap)
{
	if (render_type == VISUALIZE_TYPE::REFLECTION) return;
	//return;
	auto timer = context->list->start(L"screen");

	for (auto& eye : context->eye_context->eyes)
	{
		auto& gi_data = eye.get_or_create<EyeData>();

		gi_data.size = eye.g_buffer->size.get();
		auto& buffer = *eye.g_buffer;

		auto& list = *context->list;

		auto& graphics = context->list->get_graphics();
		buffer.result_tex.swap(context->list);
		gi_data.downsampled_light.swap(context->list);
		auto& states = screen_states[static_cast<int>(VISUALIZE_TYPE(render_type))];
		///	list.transition(downsampled_light, Render::ResourceState::RENDER_TARGET);
		list.transition(buffer.result_tex.first(), Render::ResourceState::RENDER_TARGET);
		list.transition(gi_data.current_gi_texture, Render::ResourceState::RENDER_TARGET);

		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		graphics.set_pipeline(states[0]);

		//	auto& sig = EffectSignature::signature;
			//auto &&sig = graphics.wrap<EffectSignature>();
		EffectSignature<Signature> sig(&graphics);


		sig.gi_table[0] = buffer.srv_table;
		sig.gi_table[4] = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->get_srv();
		sig.voxel_lighted[0] = volume_lighted->texture_3d()->get_srv();
		sig.camera_data = context->cam->get_const_buffer();
		sig.help_table2[0] = skymap->get_srv();
		//	if(ssgi_tex->texture_2d())
		sig.help_table2[2] = buffer.speed_tex->texture_2d()->get_srv();
		sig.help_table2[3] = buffer.result_tex.second()->texture_2d()->get_srv();

		sig.constants.set(context->voxel_min.x, context->voxel_min.y, context->voxel_min.z, context->voxel_size.x, context->voxel_size.y, context->voxel_size.z, ((context->current_time) % 10000) * 0.001f);


		{
			auto timer = context->list->start(L"downsampled");
			graphics.set_viewport(gi_data.downsampled_light.first()->texture_2d()->get_viewport(0));
			graphics.set_scissor(gi_data.downsampled_light.first()->texture_2d()->get_scissor(0));
			graphics.set_rtv(1, gi_data.downsampled_light.first()->texture_2d()->get_rtv(0), Render::Handle());

			graphics.draw(4);
		}

		{
			auto timer = context->list->start(L"filter");
			graphics.set_pipeline(states[3]);

			for (int i = 0; i < 3; i++)
			{

				gi_data.downsampled_light.swap(context->list);

				sig.help_table2[3] = gi_data.downsampled_light.second()->texture_2d()->get_srv();

				graphics.set_rtv(1, gi_data.downsampled_light.first()->texture_2d()->get_rtv(0), Render::Handle());

				graphics.draw(4);
			}
		}



		sig.help_table2[3] = buffer.result_tex.second()->texture_2d()->get_srv();


		gi_data.gi_textures.swap(context->list);

		graphics.set_viewport(buffer.result_tex.first()->texture_2d()->get_viewport(0));
		graphics.set_scissor(buffer.result_tex.first()->texture_2d()->get_scissor(0));

		gi_rtv[0].place(buffer.result_tex.first()->texture_2d()->get_rtv(0), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		gi_rtv[1].place(gi_data.gi_textures.first()->texture_2d()->get_rtv(0), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		graphics.set_rtv(gi_rtv, buffer.quality_table.dsv_table[0]);
		sig.help_table2[4] = gi_data.gi_textures.second()->texture_2d()->get_srv();
		sig.help_table2[5] = buffer.depth_tex_mips_prev->texture_2d()->get_srv();
		sig.help_table2[1] = gi_data.downsampled_light.first()->texture_2d()->get_srv();

		{
			auto timer = context->list->start(L"full");
			list.get_native_list()->OMSetStencilRef(1);
			graphics.set_pipeline(states[1]);
			graphics.draw(4);
		}


		{
			auto timer = context->list->start(L"resize");

			list.get_native_list()->OMSetStencilRef(0);
			graphics.set_pipeline(states[2]);
			graphics.draw(4);
		}


	}

}

void VoxelGI::screen_reflection(MeshRenderContext::ptr& context, scene_object::ptr scene, Render::CubemapArrayView::ptr skymap)
{
	if (!(render_type == VISUALIZE_TYPE::REFLECTION || render_type == VISUALIZE_TYPE::FULL)) return;
	auto timer = context->list->start(L"screen_reflection");

	auto& states = reflection_states[static_cast<int>(VISUALIZE_TYPE(render_type))];

	for (auto& eye : context->eye_context->eyes)
	{
		auto& gi_data = eye.get_or_create<EyeData>();

		gi_data.size = eye.g_buffer->size.get();
		auto& buffer = *eye.g_buffer;

		buffer.result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::UNORDERED_ACCESS);
		reflection_effect.process(context, buffer, skymap);
		auto& list = *context->list;

		auto& graphics = context->list->get_graphics();
		//	buffer.result_tex.swap(context->list);
		EffectSignature<Signature> sig(&graphics);


		list.transition(gi_data.downsampled_reflection, Render::ResourceState::RENDER_TARGET);
		list.transition(buffer.result_tex.first(), Render::ResourceState::RENDER_TARGET);

		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		graphics.set_signature(states.reflection_state->desc.root_signature);
		sig.gi_table[0] = buffer.srv_table;
		sig.voxel_lighted[0] = volume_lighted->texture_3d()->get_srv();
		sig.gi_table[4] = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->get_srv();

		sig.camera_data = context->cam->get_const_buffer();


		sig.help_table2[0] = skymap->get_srv();
		sig.help_table2[2] = buffer.speed_tex->texture_2d()->get_srv();
		sig.help_table2[3] = buffer.result_tex.second()->texture_2d()->get_srv();
		sig.help_table2[4] = reflection_effect.reflect_tex_dilated.second()->texture_2d()->get_srv();
		sig.help_table2[5] = gi_data.downsampled_reflection->texture_2d()->get_srv();

		sig.constants.set(context->voxel_min.x, context->voxel_min.y, context->voxel_min.z, context->voxel_size.x, context->voxel_size.y, context->voxel_size.z, ((context->current_time) % 10000) * 0.001f);

		{


			auto timer = context->list->start(L"downsampled");
			graphics.set_viewport(gi_data.downsampled_reflection->texture_2d()->get_viewport(0));
			graphics.set_scissor(gi_data.downsampled_reflection->texture_2d()->get_scissor(0));
			graphics.set_rtv(1, gi_data.downsampled_reflection->texture_2d()->get_rtv(0), Render::Handle());
			graphics.set_pipeline(downsampled_reflection_state);

			graphics.draw(4);
		}

		list.transition(gi_data.downsampled_reflection, Render::ResourceState::PIXEL_SHADER_RESOURCE);

		//	gi_textures.swap(context->list);

		graphics.set_viewport(buffer.result_tex.first()->texture_2d()->get_viewport(0));
		graphics.set_scissor(buffer.result_tex.first()->texture_2d()->get_scissor(0));

		//	gi_rtv[0].place(buffer.result_tex.first()->texture_2d()->get_rtv(0), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		//	gi_rtv[1].place(gi_textures.first()->texture_2d()->get_rtv(0), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		graphics.set_rtv(1, buffer.result_tex.first()->texture_2d()->get_rtv(0), buffer.quality_table.dsv_table[0]);



		{
			auto timer = context->list->start(L"full");
			graphics.get_native_list()->OMSetStencilRef(2);
			graphics.set_pipeline(states.reflection_state);
			graphics.draw(4);
		}
		{
			auto timer = context->list->start(L"downs");
			graphics.get_native_list()->OMSetStencilRef(0);
			graphics.set_pipeline(states.reflection_resize_state);
			graphics.draw(4);
		}

	}
	/*
	{
		auto timer = context->list->start(L"resize");

		list.get_native_list()->OMSetStencilRef(0);
		list.set_pipeline(reflection_state);
//		list.set_dynamic(4, 1, downsampled_light->texture_2d()->get_srv());
		list.draw(4);
	}
	*/
}

VoxelGI::EyeData::EyeData()
{
	size.register_change(this, [this](const ivec2& size)
		{
			if (size.x <= 0) return;
			allocator.reset();
			//downsampled_light.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, size.x/2, size.y/2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));
			gi_textures.resize(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, { size.x, size.y });
			downsampled_light.resize(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, { size.x / 2, size.y / 2 }, allocator);
			current_gi_texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE, allocator));
			downsampled_reflection.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, size.x / 2, size.y / 2, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE, allocator));

		});
}
