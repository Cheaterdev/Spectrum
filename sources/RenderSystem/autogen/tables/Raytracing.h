#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Raytracing
	{
		struct SRV
		{
			DX12::HLSL::RaytracingAccelerationStructure scene;
			DX12::HLSL::StructuredBuffer<uint> index_buffer;
		} &srv;
		DX12::HLSL::RaytracingAccelerationStructure& GetScene() { return srv.scene; }
		DX12::HLSL::StructuredBuffer<uint>& GetIndex_buffer() { return srv.index_buffer; }
		Raytracing(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
