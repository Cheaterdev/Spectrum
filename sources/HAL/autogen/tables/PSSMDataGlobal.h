#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMDataGlobal
	{
		HLSL::Texture2D<float> light_buffer;
		HLSL::StructuredBuffer<Camera> light_camera;
		HLSL::Texture2D<float>& GetLight_buffer() { return light_buffer; }
		HLSL::StructuredBuffer<Camera>& GetLight_camera() { return light_camera; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(light_buffer);
			compiler.compile(light_camera);
		}
	};
	#pragma pack(pop)
}