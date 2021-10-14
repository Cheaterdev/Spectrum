#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRendering
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float> tex0;
			Render::HLSL::Buffer<float4> positions;
		} &srv;
		Render::HLSL::Texture2D<float>& GetTex0() { return srv.tex0; }
		Render::HLSL::Buffer<float4>& GetPositions() { return srv.positions; }
		FontRendering(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
