#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshletCullData
	{
		struct CB
		{
			float4 BoundingSphere;
			uint NormalCone;
			float ApexOffset;
		} &cb;
		float4& GetBoundingSphere() { return cb.BoundingSphere; }
		uint& GetNormalCone() { return cb.NormalCone; }
		float& GetApexOffset() { return cb.ApexOffset; }
		MeshletCullData(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
