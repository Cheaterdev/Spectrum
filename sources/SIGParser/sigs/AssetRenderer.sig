
[Bind = DefaultLayout::Instance0]
struct TextureRenderer
{
	Texture2D<float4> texture;
}

PassNode AssetGBuffer
{
	[Write = {GBuffer_Albedo, GBuffer_Normals, GBuffer_Depth, GBuffer_Specular, GBuffer_Speed,
	          GBuffer_DepthMips, GBuffer_HiZ, GBuffer_HiZ_UAV}]
	GBuffer gbuffer;
	StructuredBuffer<uint> scene;
}

PassNode AssetMip
{
	GBuffer gbuffer;
	Texture ResultTexture;
	[Write] Texture swapchain;
}



Pipeline AssetPipeline
{
	ResultCreation;

	# scene prep
	PreScene;
	[Async]
	BlueNoise;

	# shadow (generate_global)
	AssetGBuffer;
	PSSM_Global;
	PSSM_Cascade;

	# sky setup (sky.generate)
	CubeSky;
	[Async]
	CubeMapDownsample;
	CubeMapEnviromentProcessor;

	# shadow composition (pssm.generate)
	PSSM_GenerateMask;
	[Async]
	PSSM_Combine;

	# sky + post
	[Async]
	Sky;
	[Async]
	SMAA;
	[Async]
	FSR;

	AssetMip;
}