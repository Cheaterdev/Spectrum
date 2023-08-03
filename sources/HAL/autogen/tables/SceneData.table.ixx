export module HAL:Autogen.Tables.SceneData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.MaterialCommandData;
import :Autogen.Tables.MeshCommandData;
import :Autogen.Tables.RaytraceInstanceInfo;
import :Autogen.Tables.node_data;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SceneData
	{
		static constexpr SlotID ID = SlotID::SceneData;
		HLSL::StructuredBuffer<node_data> nodes;
		HLSL::StructuredBuffer<MeshCommandData> meshes;
		HLSL::StructuredBuffer<MaterialCommandData> materials;
		HLSL::StructuredBuffer<RaytraceInstanceInfo> raytraceInstanceInfo;
		HLSL::StructuredBuffer<node_data>& GetNodes() { return nodes; }
		HLSL::StructuredBuffer<MeshCommandData>& GetMeshes() { return meshes; }
		HLSL::StructuredBuffer<MaterialCommandData>& GetMaterials() { return materials; }
		HLSL::StructuredBuffer<RaytraceInstanceInfo>& GetRaytraceInstanceInfo() { return raytraceInstanceInfo; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(nodes);
			compiler.compile(meshes);
			compiler.compile(materials);
			compiler.compile(raytraceInstanceInfo);
		}
		struct Compiled
		{
			uint nodes; // StructuredBuffer<node_data>
			uint meshes; // StructuredBuffer<MeshCommandData>
			uint materials; // StructuredBuffer<MaterialCommandData>
			uint raytraceInstanceInfo; // StructuredBuffer<RaytraceInstanceInfo>
		};
	};
	#pragma pack(pop)
}
