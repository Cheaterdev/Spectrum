#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsample
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> normals;
			DX12::HLSL::Texture2D<float> depth;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetNormals() { return srv.normals; }
		DX12::HLSL::Texture2D<float>& GetDepth() { return srv.depth; }
		GBufferDownsample(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
