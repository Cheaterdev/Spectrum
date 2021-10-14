#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Meshlet
	{
		struct CB
		{
			uint vertexCount;
			uint vertexOffset;
			uint primitiveCount;
			uint primitiveOffset;
		} &cb;
		uint& GetVertexCount() { return cb.vertexCount; }
		uint& GetVertexOffset() { return cb.vertexOffset; }
		uint& GetPrimitiveCount() { return cb.primitiveCount; }
		uint& GetPrimitiveOffset() { return cb.primitiveOffset; }
		Meshlet(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
