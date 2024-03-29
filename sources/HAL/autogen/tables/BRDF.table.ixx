export module HAL:Autogen.Tables.BRDF;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct BRDF
	{
		static constexpr SlotID ID = SlotID::BRDF;
		HLSL::RWTexture3D<float4> output;
		HLSL::RWTexture3D<float4>& GetOutput() { return output; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(output);
		}
		struct Compiled
		{
			uint output; // RWTexture3D<float4>
		};
	};
	#pragma pack(pop)
}
