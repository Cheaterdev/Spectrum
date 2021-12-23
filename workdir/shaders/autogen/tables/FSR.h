#pragma once
#include "FSRConstants.h"
struct FSR_cb
{
	FSRConstants_cb constants;
};
struct FSR_srv
{
	uint source; // Texture2D<float4>
};
struct FSR_uav
{
	uint target; // RWTexture2D<float4>
};
struct FSR
{
	FSR_cb cb;
	FSR_srv srv;
	FSR_uav uav;
	Texture2D<float4> GetSource() { return ResourceDescriptorHeap[srv.source]; }
	RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[uav.target]; }
	FSRConstants GetConstants() { return CreateFSRConstants(cb.constants); }

};
 const FSR CreateFSR(FSR_cb cb,FSR_srv srv,FSR_uav uav)
{
	const FSR result = {cb,srv,uav
	};
	return result;
}
