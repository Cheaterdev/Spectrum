export module HAL:Autogen.Tables.VoxelZero;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.VoxelTilingParams;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelZero
	{
		static constexpr SlotID ID = SlotID::VoxelZero;
		HLSL::RWTexture3D<float4> Target;
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>& GetTarget() { return Target; }
		VoxelTilingParams& GetParams() { return params; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(Target);
			compiler.compile(params);
		}
		struct Compiled
		{
			uint Target; // RWTexture3D<float4>
			VoxelTilingParams::Compiled params; // VoxelTilingParams
		};
	};
	#pragma pack(pop)
}
