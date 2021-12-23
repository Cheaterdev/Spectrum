#pragma once
struct Instance_cb
{
	uint instanceId; // uint
};
struct Instance
{
	Instance_cb cb;
	uint GetInstanceId() { return cb.instanceId; }

};
 const Instance CreateInstance(Instance_cb cb)
{
	const Instance result = {cb
	};
	return result;
}
