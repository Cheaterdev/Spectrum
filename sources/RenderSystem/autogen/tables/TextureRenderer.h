#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TextureRenderer
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> texture;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetTexture() { return srv.texture; }
		TextureRenderer(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
