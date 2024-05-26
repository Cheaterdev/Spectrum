#pragma once
#include "sig_hlsl.hlsl"
struct Meshlet
{
	uint vertexCount; // uint
	uint vertexOffset; // uint
	uint primitiveCount; // uint
	uint primitiveOffset; // uint
	uint GetVertexCount() { return vertexCount; }
	uint GetVertexOffset() { return vertexOffset; }
	uint GetPrimitiveCount() { return primitiveCount; }
	uint GetPrimitiveOffset() { return primitiveOffset; }
};
