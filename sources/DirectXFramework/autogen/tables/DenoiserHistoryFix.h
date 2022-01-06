#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserHistoryFix
	{
		Render::HLSL::Texture2D<float4> color;
		Render::HLSL::Texture2D<float> frames;
		Render::HLSL::RWTexture2D<float4> target;
		Render::HLSL::Texture2D<float4>& GetColor() { return color; }
		Render::HLSL::Texture2D<float>& GetFrames() { return frames; }
		Render::HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(frames);
			compiler.compile(target);
		}
	};
	#pragma pack(pop)
}
