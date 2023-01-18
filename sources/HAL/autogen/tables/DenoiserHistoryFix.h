#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserHistoryFix
	{
		HLSL::Texture2D<float4> color;
		HLSL::Texture2D<float> frames;
		HLSL::RWTexture2D<float4> target;
		HLSL::Texture2D<float4>& GetColor() { return color; }
		HLSL::Texture2D<float>& GetFrames() { return frames; }
		HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(frames);
			compiler.compile(target);
		}
		struct Compiled
		{
			uint color; // Texture2D<float4>
			uint frames; // Texture2D<float>
			uint target; // RWTexture2D<float4>
		};
	};
	#pragma pack(pop)
}
