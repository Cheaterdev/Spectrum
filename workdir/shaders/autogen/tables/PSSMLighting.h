#pragma once
#include "GBuffer.h"
struct PSSMLighting_srv
{
	Texture2D<float> light_mask;
	GBuffer_srv gbuffer;
};
struct PSSMLighting
{
	PSSMLighting_srv srv;
	Texture2D<float> GetLight_mask() { return srv.light_mask; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const PSSMLighting CreatePSSMLighting(PSSMLighting_srv srv)
{
	const PSSMLighting result = {srv
	};
	return result;
}
