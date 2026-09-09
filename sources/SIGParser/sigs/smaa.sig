

[Bind = DefaultLayout::Instance0]
struct SMAA_Global
{
	Texture2D<float4> colorTex;

	float4 subsampleIndices;
    float4 SMAA_RT_METRICS;

	RWTexture2D<float2> edgesOut;
}

[Bind = DefaultLayout::Instance1]
struct SMAA_Weights
{
	Texture2D<float4> areaTex;
	Texture2D<float4> searchTex;
	Texture2D<float4> edgesTex;

	RWTexture2D<float4> blendOut;
}


[Bind = DefaultLayout::Instance1]
struct SMAA_Blend
{
	Texture2D<float4> blendTex;

	RWTexture2D<float4> resultOut;
}





GraphicsPSO EdgeDetect
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAAEdgeDetectionVS]
	vertex = postprocess/smaa;

	[EntryPoint = DX10_SMAALumaEdgeDetectionPS]
	pixel = postprocess/smaa;

	rtv = { R8G8_UNORM };
}

GraphicsPSO BlendWeight
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAABlendingWeightCalculationVS]
	vertex = postprocess/smaa;

	[EntryPoint = DX10_SMAABlendingWeightCalculationPS]
	pixel = postprocess/smaa;

	rtv = { R8G8B8A8_UNORM };
}


GraphicsPSO Blending
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAANeighborhoodBlendingVS]
	vertex = postprocess/smaa;

	[EntryPoint = DX10_SMAANeighborhoodBlendingPS]
	pixel = postprocess/smaa;

	rtv = { R16G16B16A16_FLOAT };
}

ComputePSO EdgeDetectCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_EdgeDetect]
	compute = postprocess/smaa;
}

ComputePSO BlendWeightCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_BlendWeight]
	compute = postprocess/smaa;
}

ComputePSO BlendingCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_Blending]
	compute = postprocess/smaa;
}


[Compute]
PassNode SMAA
{
	[Write] [Recreate = ResultTextureNew]
	Texture ResultTexture;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R8G8_UNORM] Texture SMAA_edges;
	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R8G8B8A8_UNORM] Texture SMAA_blend;
}