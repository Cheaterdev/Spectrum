#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserDownsample
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> color;
			Render::HLSL::Texture2D<float> depth;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetColor() { return srv.color; }
		Render::HLSL::Texture2D<float>& GetDepth() { return srv.depth; }
		DenoiserDownsample(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
