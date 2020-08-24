[Bind = DefaultLayout::Instance2]
struct GBuffer
{
	Texture2D albedo;
	Texture2D normals;
	Texture2D specular;
	Texture2D depth;
	Texture2D<float2> motion;
}


rt GBuffer
{
	float4 albedo;
	float4 normals;
	float4 specular;
	float2 motion;

	DSV depth;
}

[Bind = DefaultLayout::Instance2]
struct GBufferDownsample
{
	Texture2D normals;
	Texture2D depth;
}



[Bind = DefaultLayout::Instance2]
struct GBufferQuality
{
	Texture2D ref;
}

[Bind = DefaultLayout::Instance0]
struct PSSMConstants
{
	int level;
	float time;
}

[Bind = DefaultLayout::Instance1]
struct PSSMData
{	
	Texture2DArray<float> light_buffer;
	StructuredBuffer<Camera> light_cameras;
}

[Bind = DefaultLayout::Instance1]
struct PSSMDataGlobal
{	
	Texture2D<float> light_buffer;
	StructuredBuffer<Camera> light_camera;
}


[Bind = DefaultLayout::Instance2]
struct PSSMLighting
{
	GBuffer gbuffer;
	Texture2D<float> light_mask;
}

