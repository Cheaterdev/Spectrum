export module HAL:Autogen.Tables.DenoiserShadow_FilterLast;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserShadow_FilterLast
	{
		static constexpr SlotID ID = SlotID::DenoiserShadow_FilterLast;
		HLSL::Texture2D<float16_t2> rqt2d_input;
		HLSL::RWTexture2D<unorm float4> rwt2d_output;
		HLSL::Texture2D<float16_t2>& GetRqt2d_input() { return rqt2d_input; }
		HLSL::RWTexture2D<unorm float4>& GetRwt2d_output() { return rwt2d_output; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(rqt2d_input);
			compiler.compile(rwt2d_output);
		}
		struct Compiled
		{
			uint rqt2d_input; // Texture2D<float16_t2>
			uint rwt2d_output; // RWTexture2D<unorm float4>
		};
	};
	#pragma pack(pop)
}
