#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMData
	{
		Render::HLSL::Texture2DArray<float> light_buffer;
		Render::HLSL::StructuredBuffer<Camera> light_cameras;
		Render::HLSL::Texture2DArray<float>& GetLight_buffer() { return light_buffer; }
		Render::HLSL::StructuredBuffer<Camera>& GetLight_cameras() { return light_cameras; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(light_buffer);
			compiler.compile(light_cameras);
		}
	};
	#pragma pack(pop)
}
