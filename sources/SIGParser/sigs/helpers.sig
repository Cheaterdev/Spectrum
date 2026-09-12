[RenderTarget]
struct SingleColor
{
	RenderTarget<float4> color;
}


[RenderTarget]
struct NoOutput
{

}

[RenderTarget]
struct DepthOnly
{
	DepthStencil<float> depth;
}

[RenderTarget]
struct SingleColorDepth
{
	RenderTarget<float4> color;
	DepthStencil<float> depth;
}


[Static]
# This pass exists solely to create() ResultTexture -- it never renders
# anything itself. [RenderCondition = `false`] (not [SetupCondition]): setup()
# still runs and creates ResultTexture every frame, it just never
# NeedsRender.
[RenderCondition = `false`]
PassNode ResultCreation
{
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture ResultTexture;
}
