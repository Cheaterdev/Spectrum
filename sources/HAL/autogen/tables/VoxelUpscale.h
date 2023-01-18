#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelUpscale
	{
		HLSL::Texture2D<float4> tex_downsampled;
		HLSL::Texture2D<float4> tex_gi_prev;
		HLSL::Texture2D<float> tex_depth_prev;
		HLSL::Texture2D<float4>& GetTex_downsampled() { return tex_downsampled; }
		HLSL::Texture2D<float4>& GetTex_gi_prev() { return tex_gi_prev; }
		HLSL::Texture2D<float>& GetTex_depth_prev() { return tex_depth_prev; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tex_downsampled);
			compiler.compile(tex_gi_prev);
			compiler.compile(tex_depth_prev);
		}
		struct Compiled
		{
			uint tex_downsampled; // Texture2D<float4>
			uint tex_gi_prev; // Texture2D<float4>
			uint tex_depth_prev; // Texture2D<float>
		};
	};
	#pragma pack(pop)
}
