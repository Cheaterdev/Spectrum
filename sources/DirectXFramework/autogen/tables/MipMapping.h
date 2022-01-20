#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MipMapping
	{
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
	};
	#pragma pack(pop)
}
