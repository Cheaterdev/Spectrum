#pragma once
using namespace Render;

//using namespace Descriptors::Realtime;

template <class T>
struct GPUMeshSignature : public T
{
	using T::T;

	struct material_data
	{
		UINT texture_offset;
		UINT node_offset;
	};

	typename T::template ConstBuffer	<0, Render::ShaderVisibility::ALL, 1>										mat_const = this;
	typename T::template ConstBuffer	<1, Render::ShaderVisibility::DOMAIN, 5>									mat_const_tess = this;
	typename T::template Table			<2, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 32>		mat_textures = this;
	typename T::template Table			<3, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::UAV, 0, 8>	mat_virtual_textures = this;
	typename T::template SRV			<4, Render::ShaderVisibility::VERTEX, 0, 1>									vertex_buffer = this;
	typename T::template ConstBuffer	<5, Render::ShaderVisibility::VERTEX, 5>									vertex_node_buffer = this;
	typename T::template ConstBuffer	<6, Render::ShaderVisibility::ALL, 0>										camera_info = this;
	typename T::template SRV			<7, Render::ShaderVisibility::VERTEX, 2, 1>									instance_buffer = this;
	typename T::template Constants		<8, Render::ShaderVisibility::ALL, 2, 2, 0, material_data>					mat_texture_offsets = this;


	typename T::template Table			<9, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::UAV, 0, 3, 1>	voxel_output = this;
	typename T::template ConstBuffer	<10, Render::ShaderVisibility::GEOMETRY, 3>									vertex_consts_geometry = this;
	typename T::template ConstBuffer	<11, Render::ShaderVisibility::PIXEL, 3>									vertex_consts_vertex = this;


	typename T::template Table			<12, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1, 1>best_fit = this;
	typename T::template Table			<13, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1, 2>best_fit2 = this;
	typename T::template SRV			<14, Render::ShaderVisibility::VERTEX, 1, 1>								vertex_unknown = this;

	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear { Render::Samplers::SamplerLinearWrapDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point { Render::Samplers::SamplerPointClampDesc, this };
	typename T::template Sampler<2, Render::ShaderVisibility::ALL, 0> aniso { Render::Samplers::SamplerAnisoWrapDesc, this };

};


template <class T>
struct GPUCacheComputeStateSignature : public T
{
	using T::T;

	typename T::template Table			<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 1>	visible_id_buffer = this;
	typename T::template SRV			<1, Render::ShaderVisibility::ALL, 1, 0>								instances = this;
	typename T::template Table			<2, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 16>	commands = this;
	typename T::template Constants		<3, Render::ShaderVisibility::ALL, 0, 3>								constants = this;

	typename T::template Table			<4, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1, 1>	seconds_dispatch = this;
	typename T::template Table			<5, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 1, 1>	from_buffer = this;
	typename T::template Table			<6, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 1, 1, 1>	target = this;	
};





template <class T>
struct GPUCacheComputeSignature : public T
{
	using T::T;

	typename T::template SRV			<0, Render::ShaderVisibility::ALL, 0, 0>								boxes_instances = this;
	typename T::template Table			<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1>	frustum_culled_commands = this;
	typename T::template Constants		<2, Render::ShaderVisibility::ALL, 0, 2>								boxes_instances_size = this;
	typename T::template Table			<3, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 1, 1>	visible_id_buffer = this;
	typename T::template ConstBuffer	<4, Render::ShaderVisibility::ALL, 1>									camera_planes = this;
	typename T::template ConstBuffer	<5, Render::ShaderVisibility::ALL, 2>									camera_info = this;
	typename T::template Table			<6, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 2, 1>	second_draw_dispatch = this;
	typename T::template Table			<7, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 3, 1>	invisible_commands = this;

};
