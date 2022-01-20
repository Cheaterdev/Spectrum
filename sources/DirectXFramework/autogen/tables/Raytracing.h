#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Raytracing
	{
		HLSL::RaytracingAccelerationStructure scene;
		HLSL::StructuredBuffer<uint> index_buffer;
		HLSL::RaytracingAccelerationStructure& GetScene() { return scene; }
		HLSL::StructuredBuffer<uint>& GetIndex_buffer() { return index_buffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(scene);
			compiler.compile(index_buffer);
		}
	};
	#pragma pack(pop)
}
