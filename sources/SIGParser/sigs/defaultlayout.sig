layout FrameLayout { 
	slot CameraData;
	slot SceneData;
	
	slot DebugInfo;
	
	Sampler linearSampler = SamplerLinearWrapDesc;
	Sampler pointClampSampler = SamplerPointClampDesc;
	Sampler linearClampSampler = SamplerLinearClampDesc;
	Sampler anisoBordeSampler = SamplerAnisoBorderDesc;

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
	uint meshes_count;

}

[Bind = FrameLayout::DebugInfo]
struct DebugInfo
{
	RWStructuredBuffer<DebugStruct> debug;
}
