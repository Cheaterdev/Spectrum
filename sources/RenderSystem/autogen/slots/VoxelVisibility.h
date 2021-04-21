#pragma once
#include "..\Tables\VoxelVisibility.h"
namespace Slots {
	struct VoxelVisibility:public DataHolder<SlotID::VoxelVisibility,Table::VoxelVisibility,DefaultLayout::Instance1>
	{
		SRV srv;
		UAV uav;
		VoxelVisibility(): DataHolder(srv,uav){}
		VoxelVisibility(const VoxelVisibility&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
