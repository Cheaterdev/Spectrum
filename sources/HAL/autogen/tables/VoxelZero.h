#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelZero
	{
		HLSL::RWTexture3D<float4> Target;
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>& GetTarget() { return Target; }
		VoxelTilingParams& GetParams() { return params; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(Target);
			compiler.compile(params);
		}
	};
	#pragma pack(pop)
}
