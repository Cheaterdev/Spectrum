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
