#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipelineGlobal
	{
		Render::HLSL::StructuredBuffer<uint> meshes_count;
		Render::HLSL::Buffer<uint> commands;
		Render::HLSL::StructuredBuffer<uint>& GetMeshes_count() { return meshes_count; }
		Render::HLSL::Buffer<uint>& GetCommands() { return commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(meshes_count);
			compiler.compile(commands);
		}
	};
	#pragma pack(pop)
}
