#pragma once
#include "..\Tables\VoxelZero.h"
namespace Slots {
	struct VoxelZero:public DataHolder<SlotID::VoxelZero,Table::VoxelZero,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		VoxelZero(): DataHolder(cb,srv,uav){}
		VoxelZero(const VoxelZero&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
