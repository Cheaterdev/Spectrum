#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelUpscale
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> tex_downsampled;
			DX12::HLSL::Texture2D<float4> tex_gi_prev;
			DX12::HLSL::Texture2D<float> tex_depth_prev;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetTex_downsampled() { return srv.tex_downsampled; }
		DX12::HLSL::Texture2D<float4>& GetTex_gi_prev() { return srv.tex_gi_prev; }
		DX12::HLSL::Texture2D<float>& GetTex_depth_prev() { return srv.tex_depth_prev; }
		VoxelUpscale(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
