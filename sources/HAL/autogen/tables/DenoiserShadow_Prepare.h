#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserShadow_Prepare
	{
		int2 BufferDimensions;
		HLSL::Texture2D<uint> t2d_hitMaskResults;
		HLSL::RWStructuredBuffer<uint> rwsb_shadowMask;
		int2& GetBufferDimensions() { return BufferDimensions; }
		HLSL::Texture2D<uint>& GetT2d_hitMaskResults() { return t2d_hitMaskResults; }
		HLSL::RWStructuredBuffer<uint>& GetRwsb_shadowMask() { return rwsb_shadowMask; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(BufferDimensions);
			compiler.compile(t2d_hitMaskResults);
			compiler.compile(rwsb_shadowMask);
		}
		struct Compiled
		{
			int2 BufferDimensions; // int2
			uint t2d_hitMaskResults; // Texture2D<uint>
			uint rwsb_shadowMask; // RWStructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
