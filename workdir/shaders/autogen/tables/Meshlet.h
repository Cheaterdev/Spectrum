#pragma once
struct Meshlet_cb
{
	uint vertexCount;
	uint vertexOffset;
	uint primitiveCount;
	uint primitiveOffset;
};
struct Meshlet
{
	Meshlet_cb cb;
	uint GetVertexCount() { return cb.vertexCount; }
	uint GetVertexOffset() { return cb.vertexOffset; }
	uint GetPrimitiveCount() { return cb.primitiveCount; }
	uint GetPrimitiveOffset() { return cb.primitiveOffset; }

};
 const Meshlet CreateMeshlet(Meshlet_cb cb)
{
	const Meshlet result = {cb
	};
	return result;
}
