#pragma once
#include "..\Tables\VoxelMipMap.h"
namespace Slots {
	struct VoxelMipMap:public DataHolder<VoxelMipMap, SlotID::VoxelMipMap,Table::VoxelMipMap,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID, DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		VoxelMipMap(): DataHolder(cb,srv,uav){}
		VoxelMipMap(const VoxelMipMap&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
