#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelCopy
	{
		Render::HLSL::Texture3D<float4> Source[2];
		Render::HLSL::RWTexture3D<float4> Target[2];
		VoxelTilingParams params;
		Render::HLSL::RWTexture3D<float4>* GetTarget() { return Target; }
		Render::HLSL::Texture3D<float4>* GetSource() { return Source; }
		VoxelTilingParams& MapParams() { return params; }
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
