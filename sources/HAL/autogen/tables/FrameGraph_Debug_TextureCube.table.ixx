export module HAL:Autogen.Tables.FrameGraph_Debug_TextureCube;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;

export namespace Table
{
	#pragma pack(push, 1)

	struct FrameGraph_Debug_TextureCube
	{
		static constexpr SlotID ID = SlotID::FrameGraph_Debug_TextureCube;
		uint2 sourceSize;
		HLSL::TextureCube<float4> source;
		HLSL::TextureCube<float4>& GetSource() { return source; }
		uint2& GetSourceSize() { return sourceSize; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(sourceSize);
			compiler.compile(source);
		}
		struct Compiled
		{
			uint2 sourceSize; // uint2
			uint source; // TextureCube<float4>

			
			private:
			SERIALIZE()
			{
				ar& NVP(sourceSize);
			}


		};

		static std::string get_typename()
		{
			return "Tables::FrameGraph_Debug_TextureCube";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(sourceSize);
		}

	};
	#pragma pack(pop)
}

