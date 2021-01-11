#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DrawStencil
	{
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<float4> vertices;
		} &srv;
		DX12::HLSL::StructuredBuffer<float4>& GetVertices() { return srv.vertices; }
		DrawStencil(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
