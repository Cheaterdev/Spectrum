#pragma once
#include "..\Tables\VoxelUtils.h"
namespace Slots {
	struct VoxelUtils:public DataHolder<Table::VoxelUtils,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		VoxelUtils(): DataHolder(cb,srv,uav){}
		VoxelUtils(const VoxelUtils&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
