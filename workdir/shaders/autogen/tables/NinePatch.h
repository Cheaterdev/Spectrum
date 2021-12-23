#pragma once
#include "vertex_input.h"
struct NinePatch_srv
{
	uint vb; // StructuredBuffer<vertex_input>
};
struct NinePatch
{
	NinePatch_srv srv;
	StructuredBuffer<vertex_input> GetVb() { return ResourceDescriptorHeap[srv.vb]; }
	Texture2D<float4> GetTextures(int i) { return bindless[i]; }

};
 const NinePatch CreateNinePatch(NinePatch_srv srv)
{
	const NinePatch result = {srv
	};
	return result;
}
