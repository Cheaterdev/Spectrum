#pragma once
#include "GBuffer.h"
struct PSSMLighting_srv
{
	uint light_mask; // Texture2D<float>
	GBuffer_srv gbuffer;
};
struct PSSMLighting
{
	PSSMLighting_srv srv;
	Texture2D<float> GetLight_mask() { return ResourceDescriptorHeap[srv.light_mask]; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const PSSMLighting CreatePSSMLighting(PSSMLighting_srv srv)
{
	const PSSMLighting result = {srv
	};
	return result;
}
