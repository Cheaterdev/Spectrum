#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelBlur
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> tex_color;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetTex_color() { return srv.tex_color; }
		VoxelBlur(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
