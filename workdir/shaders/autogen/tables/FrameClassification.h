#pragma once
struct FrameClassification_srv
{
	Texture2D<float> frames;
};
struct FrameClassification_uav
{
	AppendStructuredBuffer<uint2> hi;
	AppendStructuredBuffer<uint2> low;
};
struct FrameClassification
{
	FrameClassification_srv srv;
	FrameClassification_uav uav;
	Texture2D<float> GetFrames() { return srv.frames; }
	AppendStructuredBuffer<uint2> GetHi() { return uav.hi; }
	AppendStructuredBuffer<uint2> GetLow() { return uav.low; }

};
 const FrameClassification CreateFrameClassification(FrameClassification_srv srv,FrameClassification_uav uav)
{
	const FrameClassification result = {srv,uav
	};
	return result;
}
