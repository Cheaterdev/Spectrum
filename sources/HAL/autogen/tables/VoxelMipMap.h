#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelMipMap
	{
		HLSL::Texture3D<float4> SrcMip;
		HLSL::RWTexture3D<float4> OutMips[3];
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>* GetOutMips() { return OutMips; }
		HLSL::Texture3D<float4>& GetSrcMip() { return SrcMip; }
		VoxelTilingParams& GetParams() { return params; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(SrcMip);
			compiler.compile(OutMips);
			compiler.compile(params);
		}
	};
	#pragma pack(pop)
}