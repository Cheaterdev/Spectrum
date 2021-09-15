#pragma once
struct MeshletCullData_cb
{
	float4 BoundingSphere;
	uint NormalCone;
	float ApexOffset;
};
struct MeshletCullData
{
	MeshletCullData_cb cb;
	float4 GetBoundingSphere() { return cb.BoundingSphere; }
	uint GetNormalCone() { return cb.NormalCone; }
	float GetApexOffset() { return cb.ApexOffset; }

};
 const MeshletCullData CreateMeshletCullData(MeshletCullData_cb cb)
{
	const MeshletCullData result = {cb
	};
	return result;
}