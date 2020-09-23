#pragma once
#include "VSLine.h"
struct LineRender_srv
{
	StructuredBuffer<VSLine> vb;
};
struct LineRender
{
	LineRender_srv srv;
	StructuredBuffer<VSLine> GetVb() { return srv.vb; }

};
 const LineRender CreateLineRender(LineRender_srv srv)
{
	const LineRender result = {srv
	};
	return result;
}
