#pragma once
struct Picker_cb
{
	uint instanceId;
};
struct Picker_uav
{
	RWStructuredBuffer<uint> viewBuffer;
};
struct Picker
{
	Picker_cb cb;
	Picker_uav uav;
	uint GetInstanceId() { return cb.instanceId; }
	RWStructuredBuffer<uint> GetViewBuffer() { return uav.viewBuffer; }
};
 const Picker CreatePicker(Picker_cb cb,Picker_uav uav)
{
	const Picker result = {cb,uav
	};
	return result;
}
