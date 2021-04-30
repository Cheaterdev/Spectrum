#pragma once
#include "..\Tables\VoxelLighting.h"
namespace Slots {
	struct VoxelLighting:public DataHolder<VoxelLighting, SlotID::VoxelLighting,Table::VoxelLighting,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID, DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		VoxelLighting(): DataHolder(cb,srv,uav){}
		VoxelLighting(const VoxelLighting&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
