#pragma once
#include "PSSMDataGlobal.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelLighting
	{
		struct CB
		{
			uint groupCount;
			PSSMDataGlobal::CB pssmGlobal;
		} &cb;
		struct SRV
		{
			Render::Handle albedo;
			Render::Handle normals;
			Render::Handle lower;
			Render::Handle tex_cube;
			Render::Handle visibility;
			PSSMDataGlobal::SRV pssmGlobal;
		} &srv;
		struct UAV
		{
			Render::Handle output;
			PSSMDataGlobal::UAV pssmGlobal;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetOutput() { return uav.output; }
		Render::Handle& GetAlbedo() { return srv.albedo; }
		Render::Handle& GetNormals() { return srv.normals; }
		Render::Handle& GetLower() { return srv.lower; }
		Render::Handle& GetTex_cube() { return srv.tex_cube; }
		Render::Handle& GetVisibility() { return srv.visibility; }
		uint& GetGroupCount() { return cb.groupCount; }
		PSSMDataGlobal MapPssmGlobal() { return PSSMDataGlobal(srv.pssmGlobal); }
		VoxelLighting(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
