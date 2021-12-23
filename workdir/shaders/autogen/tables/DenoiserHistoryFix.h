#pragma once
struct DenoiserHistoryFix_srv
{
	uint color; // Texture2D<float4>
	uint frames; // Texture2D<float>
};
struct DenoiserHistoryFix_uav
{
	uint target; // RWTexture2D<float4>
};
struct DenoiserHistoryFix
{
	DenoiserHistoryFix_srv srv;
	DenoiserHistoryFix_uav uav;
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[srv.color]; }
	Texture2D<float> GetFrames() { return ResourceDescriptorHeap[srv.frames]; }
	RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[uav.target]; }

};
 const DenoiserHistoryFix CreateDenoiserHistoryFix(DenoiserHistoryFix_srv srv,DenoiserHistoryFix_uav uav)
{
	const DenoiserHistoryFix result = {srv,uav
	};
	return result;
}
