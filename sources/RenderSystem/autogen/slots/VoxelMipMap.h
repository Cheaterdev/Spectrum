#pragma once
#include "..\Tables\VoxelMipMap.h"
namespace Slots {
	struct VoxelMipMap:public DataHolder<SlotID::VoxelMipMap,Table::VoxelMipMap,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		VoxelMipMap(): DataHolder(cb,srv,uav){}
		VoxelMipMap(const VoxelMipMap&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
