#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMLighting
	{
		HLSL::Texture2D<float> light_mask;
		GBuffer gbuffer;
		HLSL::Texture2D<float>& GetLight_mask() { return light_mask; }
		GBuffer& GetGbuffer() { return gbuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(light_mask);
			compiler.compile(gbuffer);
		}
	};
	#pragma pack(pop)
}