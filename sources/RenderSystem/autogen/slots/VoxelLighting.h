#pragma once
#include "..\Tables\VoxelLighting.h"
namespace Slots {
	struct VoxelLighting:public DataHolder<SlotID::VoxelLighting,Table::VoxelLighting,DefaultLayout::Instance1>
	{
		CB cb;
		SRV srv;
		UAV uav;
		VoxelLighting(): DataHolder(cb,srv,uav){}
		VoxelLighting(const VoxelLighting&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
