#pragma once
struct MipMapping_cb
{
	uint SrcMipLevel;
	uint NumMipLevels;
	float2 TexelSize;
};
struct MipMapping_srv
{
	Texture2D<float4> SrcMip;
};
struct MipMapping_uav
{
	RWTexture2D<float4> OutMip[4];
};
struct MipMapping
{
	MipMapping_cb cb;
	MipMapping_srv srv;
	MipMapping_uav uav;
	uint GetSrcMipLevel() { return cb.SrcMipLevel; }
	uint GetNumMipLevels() { return cb.NumMipLevels; }
	float2 GetTexelSize() { return cb.TexelSize; }
	RWTexture2D<float4> GetOutMip(int i) { return uav.OutMip[i]; }
	Texture2D<float4> GetSrcMip() { return srv.SrcMip; }
};
 const MipMapping CreateMipMapping(MipMapping_cb cb,MipMapping_srv srv,MipMapping_uav uav)
{
	const MipMapping result = {cb,srv,uav
	};
	return result;
}
