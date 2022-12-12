#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameClassification
	{
		HLSL::Texture2D<float> frames;
		HLSL::AppendStructuredBuffer<uint2> hi;
		HLSL::AppendStructuredBuffer<uint2> low;
		HLSL::Texture2D<float>& GetFrames() { return frames; }
		HLSL::AppendStructuredBuffer<uint2>& GetHi() { return hi; }
		HLSL::AppendStructuredBuffer<uint2>& GetLow() { return low; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(frames);
			compiler.compile(hi);
			compiler.compile(low);
		}
		struct Compiled
		{
			uint frames; // Texture2D<float>
			uint hi; // AppendStructuredBuffer<uint2>
			uint low; // AppendStructuredBuffer<uint2>
		};
	};
#pragma pack(pop)
				}
