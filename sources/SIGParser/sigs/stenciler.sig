[Bind = DefaultLayout::Instance0]
struct Countour
{
	float4 color;
	Texture2D<float4> tex;
}

[Bind = DefaultLayout::Instance0]
struct DrawStencil
{
	StructuredBuffer<float4> vertices;
}

[Bind = DefaultLayout::Instance0]
struct PickerBuffer
{
	RWStructuredBuffer<uint> viewBuffer;
}

[Bind = DefaultLayout::Instance2]
struct Instance
{
	uint instanceId;
}


[Bind = DefaultLayout::Instance0]
struct Color
{
	float4 color;
}

