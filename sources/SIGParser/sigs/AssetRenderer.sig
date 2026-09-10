
[Bind = DefaultLayout::Instance0]
struct TextureRenderer
{
	Texture2D<float4> texture;
}

PassNode AssetGBuffer
{
	[Write = {GBuffer_Albedo, GBuffer_Normals, GBuffer_Depth, GBuffer_Specular, GBuffer_Speed,
	          GBuffer_DepthMips}]
	GBuffer gbuffer;
	# Static: occlusion pass 1 tests against LAST frame's HiZ (see SceneSystem).
	# MipCount=0: full auto mip chain (a Hi-Z pyramid), matching the original
	# manual create()'s omitted 4th Desc field.
	[Always = DepthStencil | Static] [Size = `builder.graph->get_context<Table::ViewportContext>().frame_size / 8`] [Format = R32_TYPELESS] [MipCount = 0]
	Texture GBuffer_HiZ;
	[Always = UnorderedAccess] [Size = `builder.graph->get_context<Table::ViewportContext>().frame_size / 8`] [Format = R32_FLOAT] [MipCount = 0]
	Texture GBuffer_HiZ_UAV;
	[Always = Read] StructuredBuffer<uint> scene;
}

PassNode AssetMip
{
	GBuffer gbuffer;
	[Always = Read] Texture ResultTexture;
	[Always = UnorderedAccess] Texture swapchain;
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