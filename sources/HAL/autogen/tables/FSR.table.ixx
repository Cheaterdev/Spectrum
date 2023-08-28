export module HAL:Autogen.Tables.FSR;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.FSRConstants;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FSR
	{
		static constexpr SlotID ID = SlotID::FSR;
		HLSL::Texture2D<float4> source;
		HLSL::RWTexture2D<float4> target;
		FSRConstants constants;
		HLSL::Texture2D<float4>& GetSource() { return source; }
		HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		FSRConstants& GetConstants() { return constants; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(source);
			compiler.compile(target);
			compiler.compile(constants);
		}
		struct Compiled
		{
			uint source; // Texture2D<float4>
			uint target; // RWTexture2D<float4>
			FSRConstants::Compiled constants; // FSRConstants
		};
	};
	#pragma pack(pop)
}
