#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Countour
	{
		struct CB
		{
			float4 color;
		} &cb;
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> tex;
		} &srv;
		float4& GetColor() { return cb.color; }
		DX12::HLSL::Texture2D<float4>& GetTex() { return srv.tex; }
		Countour(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
