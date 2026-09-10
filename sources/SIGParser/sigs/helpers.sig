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
# TriState: this pass exists solely to create() ResultTexture -- it never
# renders anything itself, so setup() always returns IgnoreRender rather
# than the old always-false (which conflated "disabled" with "touches
# resources but doesn't render" until create_always() could tell them apart).
[TriState]
PassNode ResultCreation
{
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture ResultTexture;
}
