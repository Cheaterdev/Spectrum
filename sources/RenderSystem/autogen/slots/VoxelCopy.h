#pragma once
#include "..\Tables\VoxelCopy.h"
namespace Slots {
	struct VoxelCopy:public DataHolder<Table::VoxelCopy,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		VoxelCopy(): DataHolder(cb,srv,uav){}
		VoxelCopy(const VoxelCopy&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
