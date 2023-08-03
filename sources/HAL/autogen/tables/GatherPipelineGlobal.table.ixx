export module HAL:Autogen.Tables.GatherPipelineGlobal;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipelineGlobal
	{
		static constexpr SlotID ID = SlotID::GatherPipelineGlobal;
		HLSL::StructuredBuffer<uint> meshes_count;
		HLSL::Buffer<uint> commands;
		HLSL::StructuredBuffer<uint>& GetMeshes_count() { return meshes_count; }
		HLSL::Buffer<uint>& GetCommands() { return commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(meshes_count);
			compiler.compile(commands);
		}
		struct Compiled
		{
			uint meshes_count; // StructuredBuffer<uint>
			uint commands; // Buffer<uint>
		};
	};
	#pragma pack(pop)
}
