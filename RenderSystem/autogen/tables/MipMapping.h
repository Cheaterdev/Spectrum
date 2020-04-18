#pragma once
namespace Table 
{
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
			Render::Handle SrcMip;
		} &srv;
		struct UAV
		{
			Render::Handle OutMip[4];
		} &uav;
		using SMP = Empty;
		uint& GetSrcMipLevel() { return cb.SrcMipLevel; }
		uint& GetNumMipLevels() { return cb.NumMipLevels; }
		float2& GetTexelSize() { return cb.TexelSize; }
		Render::Handle* GetOutMip() { return uav.OutMip; }
		Render::Handle& GetSrcMip() { return srv.SrcMip; }
		MipMapping(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
}
