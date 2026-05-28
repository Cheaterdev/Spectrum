export module HAL:Autogen.Tables.FrameGraph_Debug_Texture2D;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;

export namespace Table
{
	#pragma pack(push, 1)

	struct FrameGraph_Debug_Texture2D
	{
		static constexpr SlotID ID = SlotID::FrameGraph_Debug_Texture2D;
		uint2 sourceSize;
		float2 scale;
		float2 offset;
		HLSL::Texture2D<float4> source;
		HLSL::Texture2D<float4>& GetSource() { return source; }
		uint2& GetSourceSize() { return sourceSize; }
		float2& GetScale() { return scale; }
		float2& GetOffset() { return offset; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(sourceSize);
			compiler.compile(scale);
			compiler.compile(offset);
			compiler.compile(source);
		}
		struct Compiled
		{
			uint2 sourceSize; // uint2
			float2 scale; // float2
			float2 offset; // float2
			uint source; // Texture2D<float4>

			
			private:
			SERIALIZE()
			{
				ar& NVP(sourceSize);
				ar& NVP(scale);
				ar& NVP(offset);
			}


		};

		static std::string get_typename()
		{
			return "Tables::FrameGraph_Debug_Texture2D";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(sourceSize);
			ar& NVP(scale);
			ar& NVP(offset);
		}

	};
	#pragma pack(pop)
}

