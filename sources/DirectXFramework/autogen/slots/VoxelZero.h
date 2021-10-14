#pragma once
#include "..\Tables\VoxelZero.h"
namespace Slots {
	struct VoxelZero:public DataHolder<VoxelZero, SlotID::VoxelZero,Table::VoxelZero,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID, DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		VoxelZero(): DataHolder(cb,srv,uav){}
		VoxelZero(const VoxelZero&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
