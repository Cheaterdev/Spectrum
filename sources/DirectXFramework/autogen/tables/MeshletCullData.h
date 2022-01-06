#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshletCullData
	{
		float4 BoundingSphere;
		uint NormalCone;
		float ApexOffset;
		private:
	SERIALIZE()
		{
		     ar& NVP(BoundingSphere);
		     ar& NVP(NormalCone);
		     ar& NVP(ApexOffset);
		}
		float4& GetBoundingSphere() { return BoundingSphere; }
		uint& GetNormalCone() { return NormalCone; }
		float& GetApexOffset() { return ApexOffset; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(BoundingSphere);
			compiler.compile(NormalCone);
			compiler.compile(ApexOffset);
		}
	};
	#pragma pack(pop)
}
