#pragma once
#include "VSLine.h"
struct LineRender_srv
{
	uint vb; // StructuredBuffer<VSLine>
};
struct LineRender
{
	LineRender_srv srv;
	StructuredBuffer<VSLine> GetVb() { return ResourceDescriptorHeap[srv.vb]; }

};
 const LineRender CreateLineRender(LineRender_srv srv)
{
	const LineRender result = {srv
	};
	return result;
}
