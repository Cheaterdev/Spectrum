#pragma once
struct SMAA_Global_cb
{
	float4 subsampleIndices; // float4
	float4 SMAA_RT_METRICS; // float4
};
struct SMAA_Global_srv
{
	uint colorTex; // Texture2D<float4>
};
struct SMAA_Global
{
	SMAA_Global_cb cb;
	SMAA_Global_srv srv;
	Texture2D<float4> GetColorTex() { return ResourceDescriptorHeap[srv.colorTex]; }
	float4 GetSubsampleIndices() { return cb.subsampleIndices; }
	float4 GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }

};
 const SMAA_Global CreateSMAA_Global(SMAA_Global_cb cb,SMAA_Global_srv srv)
{
	const SMAA_Global result = {cb,srv
	};
	return result;
}
