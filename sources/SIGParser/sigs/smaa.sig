

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
	vertex = SMAA;

	[EntryPoint = DX10_SMAALumaEdgeDetectionPS]
	pixel = SMAA;

	rtv = { R8G8_UNORM };
}

GraphicsPSO BlendWeight
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAABlendingWeightCalculationVS]
	vertex = SMAA;

	[EntryPoint = DX10_SMAABlendingWeightCalculationPS]
	pixel = SMAA;

	rtv = { R8G8B8A8_UNORM };
}


GraphicsPSO Blending
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAANeighborhoodBlendingVS]
	vertex = SMAA;

	[EntryPoint = DX10_SMAANeighborhoodBlendingPS]
	pixel = SMAA;

	rtv = { R16G16B16A16_FLOAT };
}

ComputePSO EdgeDetectCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_EdgeDetect]
	compute = SMAA;
}

ComputePSO BlendWeightCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_BlendWeight]
	compute = SMAA;
}

ComputePSO BlendingCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_Blending]
	compute = SMAA;
}


[Compute]
PassNode SMAA
{
	[Write] [Recreate = ResultTextureNew]
	Texture ResultTexture;

	[Write] Texture SMAA_edges;
	[Write] Texture SMAA_blend;
}