export module HAL:Autogen.Tables.EnvFilter;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct EnvFilter
	{
		static constexpr SlotID ID = SlotID::EnvFilter;
		uint4 face;
		float4 scaler;
		uint4 size;
		uint4& GetFace() { return face; }
		float4& GetScaler() { return scaler; }
		uint4& GetSize() { return size; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(face);
			compiler.compile(scaler);
			compiler.compile(size);
		}
		using Compiled = EnvFilter;
		};
		#pragma pack(pop)
	}
