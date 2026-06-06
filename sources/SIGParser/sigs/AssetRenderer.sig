
[Bind = DefaultLayout::Instance0]
struct TextureRenderer
{
	Texture2D<float4> texture;
}

PassNode AssetGBuffer
{
	GBuffer gbuffer;
	StructuredBuffer<uint> scene;
}

PassNode AssetMip
{
	GBuffer gbuffer;
	Texture ResultTexture;
	Texture swapchain;
}



Pipeline AssetPipeline
{
	ResultCreation;

	# scene prep
	PreScene;
	BlueNoise;

	# shadow (generate_global)
	AssetGBuffer;
	PSSM_Global;
	PSSM_Cascade;

	# sky setup (sky.generate)
	CubeSky;
	CubeMapDownsample;
	CubeMapEnviromentProcessor;

	# shadow composition (pssm.generate)
	PSSM_GenerateMask;
	PSSM_Combine;

	# sky + post
	Sky;
	SMAA;
	FSR;

	AssetMip;
}