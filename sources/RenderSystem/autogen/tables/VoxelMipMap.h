#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelMipMap
	{
		struct CB
		{
			uint groupCount;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture3D<float4> SrcMip;
			Render::HLSL::StructuredBuffer<int3> visibility;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> OutMips[3];
		} &uav;
		Render::HLSL::RWTexture3D<float4>* GetOutMips() { return uav.OutMips; }
		Render::HLSL::Texture3D<float4>& GetSrcMip() { return srv.SrcMip; }
		Render::HLSL::StructuredBuffer<int3>& GetVisibility() { return srv.visibility; }
		uint& GetGroupCount() { return cb.groupCount; }
		VoxelMipMap(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
