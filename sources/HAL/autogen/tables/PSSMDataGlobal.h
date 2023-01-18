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
		struct Compiled
		{
			uint light_buffer; // Texture2D<float>
			uint light_camera; // StructuredBuffer<Camera>
		};
	};
	#pragma pack(pop)
}
