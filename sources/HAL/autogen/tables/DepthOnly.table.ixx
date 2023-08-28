export module HAL:Autogen.Tables.DepthOnly;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DepthOnly
	{
		static constexpr SlotID ID = SlotID::DepthOnly;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint depth; // DepthStencil<float>
		};
	};
	#pragma pack(pop)
}
