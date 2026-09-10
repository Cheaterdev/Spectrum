
[Bind = DefaultLayout::Instance0]
struct TextureRenderer
{
	Texture2D<float4> texture;
}

PassNode AssetGBuffer
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment on why. GBufferViewDesc::actualize() (Context.ixx) is a
	# template on `auto& context`, so it works unmodified on `data` directly
	# once these are plain top-level fields, no `.gbuffer` accessor needed.
	# GBuffer_Quality/GBuffer_DepthPrev exist only because actualize()
	# (Context.ixx) references them when bound -- both are dereference-
	# guarded there, and this pass doesn't actually read either back out of
	# the actualized GBuffer. NOT [Always=Read]: unlike Scene, AssetGBuffer
	# runs in its own isolated preview-panel graph (AssetRenderer/
	# SceneTextureRenderer's own FrameGraph::Graph, see AssetRenderer.ixx)
	# that never runs Scene's history-chain machinery, so GBuffer_DepthPrev
	# is never actually provisioned there -- auto-need()ing it unconditionally
	# crashed builder.need()'s own exists() assert the moment this pass ran.
	[Write] Texture GBuffer_Albedo;
	[Write] Texture GBuffer_Normals;
	[Write] Texture GBuffer_Depth;
	[Write] Texture GBuffer_Specular;
	[Write] Texture GBuffer_Speed;
	[Write] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
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
	# GBuffer gbuffer; used to live here, but nothing in AssetMip's own
	# render() (AssetRenderer.cpp) ever reads it -- the View's [Always]
	# leaves were being auto-need()'d for no reason. Dropped rather than
	# flattened.
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