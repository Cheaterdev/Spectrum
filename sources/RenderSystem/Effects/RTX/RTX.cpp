#include "pch.h"



void RTX::prepare(CommandList::ptr& list)
{
	rtx.prepare(list);
}

void RTX::render(ComputeContext & compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec3 size, UINT generator)
{
	PROFILE_GPU(L"raytracing");

	{
		Slots::Raytracing rtx;
		rtx.GetIndex_buffer() = universal_index_manager::get().buffer->structuredBuffer;
		rtx.GetScene() = scene_as->raytracing_handle;
		rtx.set(compute);
	}

	if (generator == 0) generator = 2;
	else if(generator == 1) generator = 3;

	compute.set_pipeline(rtx.m_dxrStateObject);
	compute.dispatch_rays<MainRTX::hit_type, shader_identifier, shader_identifier>(size,
		rtx.hitgroup_ids->buffer->get_resource_address(), rtx.hitgroup_ids->max_size(),
		rtx.miss_ids->get_resource_address(), rtx.miss_ids->get_count(),
		rtx.raygen_ids->get_resource_address().offset(generator * sizeof(raygen_type)));

}

void RTX::render_new(ComputeContext& compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec3 size)
{
	PROFILE_GPU(L"raytracing");

	{
		Slots::Raytracing rtx;
		rtx.GetIndex_buffer() = universal_index_manager::get().buffer->structuredBuffer;
		rtx.GetScene() = scene_as->raytracing_handle;
		rtx.set(compute);
	}
	
	compute.set_pipeline(rtx.m_dxrStateObject);
	compute.dispatch_rays<MainRTX::hit_type, shader_identifier, shader_identifier>(size,
		rtx.hitgroup_ids->buffer->get_resource_address(), rtx.hitgroup_ids->max_size(),
		rtx.miss_ids->get_resource_address(), rtx.miss_ids->get_count() ,
		rtx.raygen_ids->get_resource_address().offset(0*sizeof(raygen_type)));
	
	
}