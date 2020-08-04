#pragma once
#include "..\Tables\Voxelization.h"
namespace Slots {
	struct Voxelization:public DataHolder<Table::Voxelization,DefaultLayout::Instance2>
	{
		CB cb;
		UAV uav;
		Voxelization(): DataHolder(cb,uav){}
		Voxelization(const Voxelization&other): DataHolder(cb,uav){cb = other.cb;uav = other.uav;}
	};
}
