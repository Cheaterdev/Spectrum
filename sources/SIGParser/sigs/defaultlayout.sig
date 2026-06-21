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
	slot Instance3;
	slot Instance4;
	slot Instance5;


	slot Raytracing;

	slot MaterialData;

	slot WorkGR_ClassifyPixels_NodeEmulation;
	slot WorkGR_Shadows_NodeEmulation;
}

struct DebugStruct
{
	uint4 v;
}

layout NoneLayout { 
	slot None;
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

            GetDebug()[id] = debug;
		}
		
	}%
}
