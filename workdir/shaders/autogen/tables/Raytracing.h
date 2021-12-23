#pragma once
struct Raytracing_srv
{
	uint scene; // RaytracingAccelerationStructure
	uint index_buffer; // StructuredBuffer<uint>
};
struct Raytracing
{
	Raytracing_srv srv;
	RaytracingAccelerationStructure GetScene() { return ResourceDescriptorHeap[srv.scene]; }
	StructuredBuffer<uint> GetIndex_buffer() { return ResourceDescriptorHeap[srv.index_buffer]; }

};
 const Raytracing CreateRaytracing(Raytracing_srv srv)
{
	const Raytracing result = {srv
	};
	return result;
}
