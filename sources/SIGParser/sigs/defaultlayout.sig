layout FrameLayout { 
	slot CameraData;
	slot SceneData;

	
	Sampler linearSampler = SamplerLinearWrapDesc;
	Sampler pointClampSampler = SamplerPointClampDesc;
	Sampler linearClampSampler = SamplerLinearClampDesc;
}

layout DefaultLayout: FrameLayout 
{
	slot Instance0;
	slot Instance1;
	slot Instance2;


	slot Raytracing;

	slot MaterialData;
}