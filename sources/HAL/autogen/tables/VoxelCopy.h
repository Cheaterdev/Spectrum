#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelCopy
	{
		HLSL::Texture3D<float4> Source[2];
		HLSL::RWTexture3D<float4> Target[2];
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>* GetTarget() { return Target; }
		HLSL::Texture3D<float4>* GetSource() { return Source; }
		VoxelTilingParams& GetParams() { return params; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(Source);
			compiler.compile(Target);
			compiler.compile(params);
		}
	};
	#pragma pack(pop)
}
