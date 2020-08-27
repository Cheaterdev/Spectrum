
[Bind = DefaultLayout::Instance2]
struct RaytracingRays
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
}



[Bind = DefaultLayout::Raytracing]
struct Raytracing
{
	RaytracingAccelerationStructure scene;
	StructuredBuffer<uint> index_buffer;
}
