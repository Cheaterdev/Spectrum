#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Raytracing
	{
		struct SRV
		{
			Render::HLSL::RaytracingAccelerationStructure scene;
			Render::HLSL::StructuredBuffer<uint> index_buffer;
		} &srv;
		Render::HLSL::RaytracingAccelerationStructure& GetScene() { return srv.scene; }
		Render::HLSL::StructuredBuffer<uint>& GetIndex_buffer() { return srv.index_buffer; }
		Raytracing(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
