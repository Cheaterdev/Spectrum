#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameClassification
	{
		Render::HLSL::Texture2D<float> frames;
		Render::HLSL::AppendStructuredBuffer<uint2> hi;
		Render::HLSL::AppendStructuredBuffer<uint2> low;
		Render::HLSL::Texture2D<float>& GetFrames() { return frames; }
		Render::HLSL::AppendStructuredBuffer<uint2>& GetHi() { return hi; }
		Render::HLSL::AppendStructuredBuffer<uint2>& GetLow() { return low; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(frames);
			compiler.compile(hi);
			compiler.compile(low);
		}
	};
	#pragma pack(pop)
}
