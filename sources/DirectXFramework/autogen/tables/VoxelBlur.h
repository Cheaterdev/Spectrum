#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelBlur
	{
		Render::HLSL::Texture2D<float4> noisy_output;
		Render::HLSL::Texture2D<float4> hit_and_pdf;
		Render::HLSL::Texture2D<float4> prev_result;
		Render::HLSL::RWTexture2D<float4> screen_result;
		Render::HLSL::RWTexture2D<float4> gi_result;
		Render::HLSL::Texture2D<float4>& GetNoisy_output() { return noisy_output; }
		Render::HLSL::Texture2D<float4>& GetHit_and_pdf() { return hit_and_pdf; }
		Render::HLSL::Texture2D<float4>& GetPrev_result() { return prev_result; }
		Render::HLSL::RWTexture2D<float4>& GetScreen_result() { return screen_result; }
		Render::HLSL::RWTexture2D<float4>& GetGi_result() { return gi_result; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(noisy_output);
			compiler.compile(hit_and_pdf);
			compiler.compile(prev_result);
			compiler.compile(screen_result);
			compiler.compile(gi_result);
		}
	};
	#pragma pack(pop)
}
