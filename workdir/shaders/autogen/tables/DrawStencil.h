#pragma once
struct DrawStencil_srv
{
	StructuredBuffer<float4> vertices;
};
struct DrawStencil
{
	DrawStencil_srv srv;
	StructuredBuffer<float4> GetVertices() { return srv.vertices; }

};
 const DrawStencil CreateDrawStencil(DrawStencil_srv srv)
{
	const DrawStencil result = {srv
	};
	return result;
}
