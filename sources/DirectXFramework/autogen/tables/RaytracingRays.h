#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct RaytracingRays
	{
		float pixelAngle;
		Render::HLSL::RWTexture2D<float4> output;
		GBuffer gbuffer;
		Render::HLSL::RWTexture2D<float4>& GetOutput() { return output; }
		float& GetPixelAngle() { return pixelAngle; }
		GBuffer& GetGbuffer() { return gbuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pixelAngle);
			compiler.compile(output);
			compiler.compile(gbuffer);
		}
	};
	#pragma pack(pop)
}
