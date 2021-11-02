#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct mesh_vertex_input
	{
		struct CB
		{
			float3 pos;
			float3 normal;
			float2 tc;
			float4 tangent;
			private:
	SERIALIZE()
			{
			     ar& NVP(pos);
			     ar& NVP(normal);
			     ar& NVP(tc);
			     ar& NVP(tangent);
			}
		} &cb;
		float3& GetPos() { return cb.pos; }
		float3& GetNormal() { return cb.normal; }
		float2& GetTc() { return cb.tc; }
		float4& GetTangent() { return cb.tangent; }
		mesh_vertex_input(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
