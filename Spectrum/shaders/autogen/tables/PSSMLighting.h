#pragma once
#include "GBuffer.h"
struct PSSMLighting_srv
{
	Texture2D<float> light_mask;
	Texture3D<float4> brdf;
	GBuffer_srv gbuffer;
};
struct PSSMLighting
{
	PSSMLighting_srv srv;
	Texture2D<float> GetLight_mask() { return srv.light_mask; }
	Texture3D<float4> GetBrdf() { return srv.brdf; }
	GBuffer gbuffer;
	GBuffer GetGbuffer() { return gbuffer; }
};
 const PSSMLighting CreatePSSMLighting(PSSMLighting_srv srv)
{
	const PSSMLighting result = {srv
	, CreateGBuffer(srv.gbuffer)
	};
	return result;
}
