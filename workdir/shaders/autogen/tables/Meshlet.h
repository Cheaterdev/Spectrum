#pragma once
struct Meshlet_cb
{
	uint vertexCount; // uint
	uint vertexOffset; // uint
	uint primitiveCount; // uint
	uint primitiveOffset; // uint
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
