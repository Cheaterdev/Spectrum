#pragma once
#include "..\Tables\VoxelCopy.h"
namespace Slots {
	struct VoxelCopy:public DataHolder<VoxelCopy, SlotID::VoxelCopy,Table::VoxelCopy,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID, DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		VoxelCopy(): DataHolder(cb,srv,uav){}
		VoxelCopy(const VoxelCopy&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
