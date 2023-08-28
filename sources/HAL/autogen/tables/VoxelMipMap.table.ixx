export module HAL:Autogen.Tables.VoxelMipMap;
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
	struct VoxelMipMap
	{
		static constexpr SlotID ID = SlotID::VoxelMipMap;
		HLSL::Texture3D<float4> SrcMip;
		HLSL::RWTexture3D<float4> OutMips[3];
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>* GetOutMips() { return OutMips; }
		HLSL::Texture3D<float4>& GetSrcMip() { return SrcMip; }
		VoxelTilingParams& GetParams() { return params; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(SrcMip);
			compiler.compile(OutMips);
			compiler.compile(params);
		}
		struct Compiled
		{
			uint SrcMip; // Texture3D<float4>
			uint OutMips[3]; // RWTexture3D<float4>
			VoxelTilingParams::Compiled params; // VoxelTilingParams
		};
	};
	#pragma pack(pop)
}
