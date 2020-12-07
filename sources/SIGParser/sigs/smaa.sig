

[Bind = DefaultLayout::Instance0]
struct SMAA_Global
{
	Texture2D<float4> colorTex;
	
	float4 subsampleIndices;
    float4 SMAA_RT_METRICS;
}

[Bind = DefaultLayout::Instance1]
struct SMAA_Weights
{
	Texture2D<float4> areaTex;
	Texture2D<float4> searchTex;
	Texture2D<float4> edgesTex;
}


[Bind = DefaultLayout::Instance1]
struct SMAA_Blend
{
	Texture2D<float4> blendTex;
}





GraphicsPSO EdgeDetect
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAAEdgeDetectionVS]
	vertex = SMAA;

	[EntryPoint = DX10_SMAALumaEdgeDetectionPS]
	pixel = SMAA;

	rtv = { DXGI_FORMAT_R8G8_UNORM };
}

GraphicsPSO BlendWeight
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAABlendingWeightCalculationVS]
	vertex = SMAA;

	[EntryPoint = DX10_SMAABlendingWeightCalculationPS]
	pixel = SMAA;

	rtv = { DXGI_FORMAT_R8G8B8A8_UNORM };
}


GraphicsPSO Blending
{
	root = DefaultLayout;

	[EntryPoint = DX10_SMAANeighborhoodBlendingVS]
	vertex = SMAA;

	[EntryPoint = DX10_SMAANeighborhoodBlendingPS]
	pixel = SMAA;

	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT };
}
