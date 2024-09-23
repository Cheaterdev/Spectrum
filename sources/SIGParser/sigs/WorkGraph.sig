
[Bind = DefaultLayout::Instance0]
struct WorkGraphTest
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
	RWTexture2D<float4> target;
}


struct GraphInput
{
	[DispatchSize]
	uint3 dispatch_grid;
}


