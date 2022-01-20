#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelDebug
	{
		HLSL::Texture3D<float4> volume;
		GBuffer gbuffer;
		HLSL::Texture3D<float4>& GetVolume() { return volume; }
		GBuffer& GetGbuffer() { return gbuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(volume);
			compiler.compile(gbuffer);
		}
	};
	#pragma pack(pop)
}
