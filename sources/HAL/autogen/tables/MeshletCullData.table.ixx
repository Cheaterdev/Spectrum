export module HAL:Autogen.Tables.MeshletCullData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct MeshletCullData
	{
		static constexpr SlotID ID = SlotID::MeshletCullData;
		float4 BoundingSphere;
		uint NormalCone;
		float ApexOffset;
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
		using Compiled = MeshletCullData;
		private:
	SERIALIZE()
		{
		     ar& NVP(BoundingSphere);
		     ar& NVP(NormalCone);
		     ar& NVP(ApexOffset);
		}
		};
		#pragma pack(pop)
	}