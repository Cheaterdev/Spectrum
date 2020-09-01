

[Bind = DefaultLayout::Instance0]
struct SMAA_Global
{
	Texture2D colorTex;
	
	float4 subsampleIndices;
    float4 SMAA_RT_METRICS;
}

[Bind = DefaultLayout::Instance1]
struct SMAA_Weights
{
	Texture2D areaTex;
	Texture2D searchTex;
	Texture2D edgesTex;
}


[Bind = DefaultLayout::Instance1]
struct SMAA_Blend
{
	Texture2D blendTex;
}

