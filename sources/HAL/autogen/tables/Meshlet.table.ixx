export module HAL:Autogen.Tables.Meshlet;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Meshlet
	{
		static constexpr SlotID ID = SlotID::Meshlet;
		uint vertexCount;
		uint vertexOffset;
		uint primitiveCount;
		uint primitiveOffset;
		uint& GetVertexCount() { return vertexCount; }
		uint& GetVertexOffset() { return vertexOffset; }
		uint& GetPrimitiveCount() { return primitiveCount; }
		uint& GetPrimitiveOffset() { return primitiveOffset; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertexCount);
			compiler.compile(vertexOffset);
			compiler.compile(primitiveCount);
			compiler.compile(primitiveOffset);
		}
		using Compiled = Meshlet;
		};
		#pragma pack(pop)
	}
