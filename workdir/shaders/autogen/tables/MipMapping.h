#pragma once
struct MipMapping_cb
{
	uint SrcMipLevel; // uint
	uint NumMipLevels; // uint
	float2 TexelSize; // float2
};
struct MipMapping_srv
{
	uint SrcMip; // Texture2D<float4>
};
struct MipMapping_uav
{
	uint OutMip[4]; // RWTexture2D<float4>
};
struct MipMapping
{
	MipMapping_cb cb;
	MipMapping_srv srv;
	MipMapping_uav uav;
	RWTexture2D<float4> GetOutMip(int i) { return ResourceDescriptorHeap[uav.OutMip[i]]; }
	Texture2D<float4> GetSrcMip() { return ResourceDescriptorHeap[srv.SrcMip]; }
	uint GetSrcMipLevel() { return cb.SrcMipLevel; }
	uint GetNumMipLevels() { return cb.NumMipLevels; }
	float2 GetTexelSize() { return cb.TexelSize; }

};
 const MipMapping CreateMipMapping(MipMapping_cb cb,MipMapping_srv srv,MipMapping_uav uav)
{
	const MipMapping result = {cb,srv,uav
	};
	return result;
}
