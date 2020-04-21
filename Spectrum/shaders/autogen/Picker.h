#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Picker.h"
ConstantBuffer<Picker_cb> cb_2_0:register(b0,space2);
RWStructuredBuffer<uint> uav_2_0: register(u0, space2);
Picker CreatePicker()
{
	Picker result;
	result.cb = cb_2_0;
	result.uav.viewBuffer = uav_2_0;
	return result;
}
static const Picker picker_global = CreatePicker();
const Picker GetPicker(){ return picker_global; }
