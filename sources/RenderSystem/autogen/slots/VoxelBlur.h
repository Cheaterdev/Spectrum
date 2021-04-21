#pragma once
#include "..\Tables\VoxelBlur.h"
namespace Slots {
	struct VoxelBlur:public DataHolder<SlotID::VoxelBlur,Table::VoxelBlur,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		VoxelBlur(): DataHolder(srv,uav){}
		VoxelBlur(const VoxelBlur&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
