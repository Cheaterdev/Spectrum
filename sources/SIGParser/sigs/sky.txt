[Bind = DefaultLayout::Instance0]
struct SkyData
{

	float3 sunDir;
	Texture2D depthBuffer;
 
	Texture2D transmittance;
	Texture3D inscatter;
	Texture2D irradiance;
}


[Bind = DefaultLayout::Instance1]
struct SkyFace
{
	uint face;
}


[Bind = DefaultLayout::Instance1]
struct EnvFilter
{
	uint4 face;	
	float4 scaler;
	uint4 size;
}


[Bind = DefaultLayout::Instance2]
struct EnvSource
{
	TextureCube sourceTex;
}


