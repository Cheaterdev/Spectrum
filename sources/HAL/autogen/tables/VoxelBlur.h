#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelBlur
	{
		static constexpr SlotID ID = SlotID::VoxelBlur;
		HLSL::Texture2D<float4> noisy_output;
		HLSL::Texture2D<float4> hit_and_pdf;
		HLSL::Texture2D<float4> prev_result;
		HLSL::RWTexture2D<float4> screen_result;
		HLSL::RWTexture2D<float4> gi_result;
		HLSL::Texture2D<float4>& GetNoisy_output() { return noisy_output; }
		HLSL::Texture2D<float4>& GetHit_and_pdf() { return hit_and_pdf; }
		HLSL::Texture2D<float4>& GetPrev_result() { return prev_result; }
		HLSL::RWTexture2D<float4>& GetScreen_result() { return screen_result; }
		HLSL::RWTexture2D<float4>& GetGi_result() { return gi_result; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(noisy_output);
			compiler.compile(hit_and_pdf);
			compiler.compile(prev_result);
			compiler.compile(screen_result);
			compiler.compile(gi_result);
		}
		struct Compiled
		{
			uint noisy_output; // Texture2D<float4>
			uint hit_and_pdf; // Texture2D<float4>
			uint prev_result; // Texture2D<float4>
			uint screen_result; // RWTexture2D<float4>
			uint gi_result; // RWTexture2D<float4>
		};
	};
	#pragma pack(pop)
}
