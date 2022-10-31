#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Meshlet
	{
		uint vertexCount;
		uint vertexOffset;
		uint primitiveCount;
		uint primitiveOffset;
		uint& GetVertexCount() { return vertexCount; }
		uint& GetVertexOffset() { return vertexOffset; }
		uint& GetPrimitiveCount() { return primitiveCount; }
		uint& GetPrimitiveOffset() { return primitiveOffset; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertexCount);
			compiler.compile(vertexOffset);
			compiler.compile(primitiveCount);
			compiler.compile(primitiveOffset);
		}
	};
	#pragma pack(pop)
}