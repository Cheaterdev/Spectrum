#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRendering
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float> tex0;
			DX12::HLSL::Buffer<float4> positions;
		} &srv;
		DX12::HLSL::Texture2D<float>& GetTex0() { return srv.tex0; }
		DX12::HLSL::Buffer<float4>& GetPositions() { return srv.positions; }
		FontRendering(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
