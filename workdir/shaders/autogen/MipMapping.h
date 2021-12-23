#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MipMapping.h"
ConstantBuffer<MipMapping_cb> cb_4_0:register(b0,space4);
Texture2D<float4> srv_4_0: register(t0, space4);
RWTexture2D<float4> uav_4_0[4]: register(u0, space4);
struct Pass_MipMapping
{
uint srv_0;
uint uav_0;
};
ConstantBuffer<Pass_MipMapping> pass_MipMapping: register( b2, space4);
const MipMapping CreateMipMapping()
{
	MipMapping result;
	result.cb = cb_4_0;
	result.srv.SrcMip = (pass_MipMapping.srv_0 );
	result.uav.OutMip[0] = (pass_MipMapping.uav_0) + 0; 
	result.uav.OutMip[1] = (pass_MipMapping.uav_0) + 1; 
	result.uav.OutMip[2] = (pass_MipMapping.uav_0) + 2; 
	result.uav.OutMip[3] = (pass_MipMapping.uav_0) + 3; 
	return result;
}
#ifndef NO_GLOBAL
static const MipMapping mipMapping_global = CreateMipMapping();
const MipMapping GetMipMapping(){ return mipMapping_global; }
#endif
