#pragma once
#include "Materials/universal_material.h"
#include "Assets/MeshAsset.h"

class RTX :public Singleton<RTX>, Events::prop_handler,
	public Events::Runner
{

public:
	
	using ptr = std::shared_ptr<RTX>;


	MainRTX rtx;

	void prepare(CommandList::ptr& list);


	template<class T>
	void render(ComputeContext& compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec2 size)
	{
		render<T>(compute, scene_as, ivec3{size,1});
	}


	template<class T>
	void render(ComputeContext& compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec3 size)
	{

		if (!Render::Device::get().is_rtx_supported()) return;
		
		{
			Slots::Raytracing rtx;
			rtx.GetIndex_buffer() = universal_index_manager::get().buffer->structuredBuffer;
			rtx.GetScene() = scene_as->raytracing_handle;
			rtx.set(compute);
		}

		if(!GetAsyncKeyState('8'))
		rtx.dispatch<T>(size, compute);
	}


};
