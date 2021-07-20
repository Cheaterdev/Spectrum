#pragma once
#include "FSRConstants.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FSR
	{
		struct CB
		{
			FSRConstants::CB constants;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture2D<float4> source;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture2D<float4> target;
		} &uav;
		Render::HLSL::Texture2D<float4>& GetSource() { return srv.source; }
		Render::HLSL::RWTexture2D<float4>& GetTarget() { return uav.target; }
		FSRConstants MapConstants() { return FSRConstants(cb.constants); }
		FSR(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
