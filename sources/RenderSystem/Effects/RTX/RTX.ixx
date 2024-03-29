export module Graphics:RTX;

import :MeshAsset;

import HAL;
import Core;

export class RTX :public Singleton<RTX>, Events::prop_handler,
	public Events::Runner
{

public:
	
	using ptr = std::shared_ptr<RTX>;


	MainRTX rtx;

	void prepare(HAL::CommandList::ptr& list);


	template<class T>
	void render(HAL::ComputeContext& compute, HAL::RaytracingAccelerationStructure::ptr scene_as, ivec2 size)
	{
		render<T>(compute, scene_as, ivec3{size,1});
	}


	template<class T>
	void render(HAL::ComputeContext& compute, HAL::RaytracingAccelerationStructure::ptr scene_as, ivec3 size)
	{

		if (!HAL::Device::get().is_rtx_supported()) return;
		
		{
			Slots::Raytracing rtx;
			rtx.GetScene() = scene_as->raytracing_handle;
			compute.set(rtx);
		}

	
		rtx.dispatch<T>(size, compute);
	}


};
