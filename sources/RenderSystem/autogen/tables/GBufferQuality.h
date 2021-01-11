#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferQuality
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> ref;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetRef() { return srv.ref; }
		GBufferQuality(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
