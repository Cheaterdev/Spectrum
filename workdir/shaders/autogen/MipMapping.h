#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MipMapping.h"
ConstantBuffer<MipMapping_cb> cb_3_0:register(b0,space3);
Texture2D<float4> srv_3_0: register(t0, space3);
RWTexture2D<float4> uav_3_0[4]: register(u0, space3);
MipMapping CreateMipMapping()
{
	MipMapping result;
	result.cb = cb_3_0;
	result.srv.SrcMip = srv_3_0;
	result.uav.OutMip = uav_3_0;
	return result;
}
static const MipMapping mipMapping_global = CreateMipMapping();
const MipMapping GetMipMapping(){ return mipMapping_global; }
