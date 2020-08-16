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
			Render::Handle SrcMip;
			Render::Handle visibility;
		} &srv;
		struct UAV
		{
			Render::Handle OutMips[3];
		} &uav;
		using SMP = Empty;
		Render::Handle* GetOutMips() { return uav.OutMips; }
		Render::Handle& GetSrcMip() { return srv.SrcMip; }
		Render::Handle& GetVisibility() { return srv.visibility; }
		uint& GetGroupCount() { return cb.groupCount; }
		VoxelMipMap(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
