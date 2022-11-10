#pragma once
struct MeshletCullData
{
	float4 BoundingSphere; // float4
	uint NormalCone; // uint
	float ApexOffset; // float
	float4 GetBoundingSphere() { return BoundingSphere; }
	uint GetNormalCone() { return NormalCone; }
	float GetApexOffset() { return ApexOffset; }
};
