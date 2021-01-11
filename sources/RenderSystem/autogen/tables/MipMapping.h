#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MipMapping
	{
		struct CB
		{
			uint SrcMipLevel;
			uint NumMipLevels;
			float2 TexelSize;
		} &cb;
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> SrcMip;
		} &srv;
		struct UAV
		{
			DX12::HLSL::RWTexture2D<float4> OutMip[4];
		} &uav;
		uint& GetSrcMipLevel() { return cb.SrcMipLevel; }
		uint& GetNumMipLevels() { return cb.NumMipLevels; }
		float2& GetTexelSize() { return cb.TexelSize; }
		DX12::HLSL::RWTexture2D<float4>* GetOutMip() { return uav.OutMip; }
		DX12::HLSL::Texture2D<float4>& GetSrcMip() { return srv.SrcMip; }
		MipMapping(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
