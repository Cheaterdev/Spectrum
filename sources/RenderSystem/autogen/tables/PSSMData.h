#pragma once
#include "Camera.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMData
	{
		struct SRV
		{
			DX12::HLSL::Texture2DArray<float> light_buffer;
			DX12::HLSL::StructuredBuffer<Camera> light_cameras;
		} &srv;
		DX12::HLSL::Texture2DArray<float>& GetLight_buffer() { return srv.light_buffer; }
		DX12::HLSL::StructuredBuffer<Camera>& GetLight_cameras() { return srv.light_cameras; }
		PSSMData(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
