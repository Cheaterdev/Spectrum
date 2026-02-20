export module HAL:Autogen.Tables.Meshlet;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
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

		static std::string get_typename()
		{
			return "Tables::Meshlet";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(vertexCount);
			ar& NVP(vertexOffset);
			ar& NVP(primitiveCount);
			ar& NVP(primitiveOffset);
		}

	};
	#pragma pack(pop)
}

