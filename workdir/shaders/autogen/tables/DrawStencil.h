#pragma once
struct DrawStencil_srv
{
	uint vertices; // StructuredBuffer<float4>
};
struct DrawStencil
{
	DrawStencil_srv srv;
	StructuredBuffer<float4> GetVertices() { return ResourceDescriptorHeap[srv.vertices]; }

};
 const DrawStencil CreateDrawStencil(DrawStencil_srv srv)
{
	const DrawStencil result = {srv
	};
	return result;
}
