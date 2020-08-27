#pragma once
struct Raytracing_srv
{
	RaytracingAccelerationStructure scene;
	StructuredBuffer<uint> index_buffer;
};
struct Raytracing
{
	Raytracing_srv srv;
	RaytracingAccelerationStructure GetScene() { return srv.scene; }
	StructuredBuffer<uint> GetIndex_buffer() { return srv.index_buffer; }

};
 const Raytracing CreateRaytracing(Raytracing_srv srv)
{
	const Raytracing result = {srv
	};
	return result;
}
