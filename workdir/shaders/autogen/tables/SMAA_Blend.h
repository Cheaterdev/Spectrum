#pragma once
struct SMAA_Blend_srv
{
	uint blendTex; // Texture2D<float4>
};
struct SMAA_Blend
{
	SMAA_Blend_srv srv;
	Texture2D<float4> GetBlendTex() { return ResourceDescriptorHeap[srv.blendTex]; }

};
 const SMAA_Blend CreateSMAA_Blend(SMAA_Blend_srv srv)
{
	const SMAA_Blend result = {srv
	};
	return result;
}
