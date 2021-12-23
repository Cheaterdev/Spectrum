#pragma once
struct FrameClassification_srv
{
	uint frames; // Texture2D<float>
};
struct FrameClassification_uav
{
	uint hi; // AppendStructuredBuffer<uint2>
	uint low; // AppendStructuredBuffer<uint2>
};
struct FrameClassification
{
	FrameClassification_srv srv;
	FrameClassification_uav uav;
	Texture2D<float> GetFrames() { return ResourceDescriptorHeap[srv.frames]; }
	AppendStructuredBuffer<uint2> GetHi() { return ResourceDescriptorHeap[uav.hi]; }
	AppendStructuredBuffer<uint2> GetLow() { return ResourceDescriptorHeap[uav.low]; }

};
 const FrameClassification CreateFrameClassification(FrameClassification_srv srv,FrameClassification_uav uav)
{
	const FrameClassification result = {srv,uav
	};
	return result;
}
