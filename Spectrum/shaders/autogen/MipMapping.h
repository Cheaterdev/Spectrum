#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MipMapping.h"
ConstantBuffer<MipMapping_cb> cb_2_0:register(b0,space2);
Texture2D<float4> srv_2_0: register(t0, space2);
RWTexture2D<float4> uav_2_0[4]: register(u0, space2);
MipMapping CreateMipMapping()
{
	MipMapping result;
	result.cb = cb_2_0;
	result.srv.SrcMip = srv_2_0;
	result.uav.OutMip = uav_2_0;
	return result;
}
static const MipMapping mipMapping_global = CreateMipMapping();
const MipMapping GetMipMapping(){ return mipMapping_global; }
