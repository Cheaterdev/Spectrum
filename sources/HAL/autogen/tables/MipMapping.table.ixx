export module HAL:Autogen.Tables.MipMapping;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct MipMapping
	{
		static constexpr SlotID ID = SlotID::MipMapping;
		uint SrcMipLevel;
		uint NumMipLevels;
		float2 TexelSize;
		HLSL::Texture2D<float4> SrcMip;
		HLSL::RWTexture2D<float4> OutMip[4];
		uint& GetSrcMipLevel() { return SrcMipLevel; }
		uint& GetNumMipLevels() { return NumMipLevels; }
		float2& GetTexelSize() { return TexelSize; }
		HLSL::RWTexture2D<float4>* GetOutMip() { return OutMip; }
		HLSL::Texture2D<float4>& GetSrcMip() { return SrcMip; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(SrcMipLevel);
			compiler.compile(NumMipLevels);
			compiler.compile(TexelSize);
			compiler.compile(SrcMip);
			compiler.compile(OutMip);
		}
		struct Compiled
		{
			uint SrcMipLevel; // uint
			uint NumMipLevels; // uint
			float2 TexelSize; // float2
			uint SrcMip; // Texture2D<float4>
			uint OutMip[4]; // RWTexture2D<float4>
		};
	};
	#pragma pack(pop)
}
