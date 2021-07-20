#pragma once
#include "FSRConstants.h"
struct FSR_cb
{
	FSRConstants_cb constants;
};
struct FSR_srv
{
	Texture2D<float4> source;
};
struct FSR_uav
{
	RWTexture2D<float4> target;
};
struct FSR
{
	FSR_cb cb;
	FSR_srv srv;
	FSR_uav uav;
	Texture2D<float4> GetSource() { return srv.source; }
	RWTexture2D<float4> GetTarget() { return uav.target; }
	FSRConstants GetConstants() { return CreateFSRConstants(cb.constants); }

};
 const FSR CreateFSR(FSR_cb cb,FSR_srv srv,FSR_uav uav)
{
	const FSR result = {cb,srv,uav
	};
	return result;
}
