layout FrameLayout { 
	slot CameraData;
	slot SceneData;
	slot PassData;

	slot DebugInfo;
	
	Sampler linearSampler = SamplerLinearWrapDesc;
	Sampler pointClampSampler = SamplerPointClampDesc;
	Sampler linearClampSampler = SamplerLinearClampDesc;
	Sampler anisoBordeSampler = SamplerAnisoBorderDesc;
	Sampler pointBorderSampler = SamplerPointBorderDesc;
}

layout DefaultLayout: FrameLayout 
{
	slot Instance0;
	slot Instance1;
	slot Instance2;


	slot Raytracing;

	slot MaterialData;
}

struct DebugStruct
{
	uint4 v;
}

[Bind = FrameLayout::DebugInfo]
struct DebugInfo
{
	RWStructuredBuffer<DebugStruct> debug;

	%{
		void Log(uint id, uint4 v)
		{
			DebugStruct debug;

            debug.v = v;

            uav.debug[id] = debug;
		}
		
	}%
}
