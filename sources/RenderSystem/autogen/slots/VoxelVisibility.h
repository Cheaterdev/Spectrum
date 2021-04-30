#pragma once
#include "..\Tables\VoxelVisibility.h"
namespace Slots {
	struct VoxelVisibility:public DataHolder<VoxelVisibility, SlotID::VoxelVisibility,Table::VoxelVisibility,DefaultLayout::Instance1>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		VoxelVisibility(): DataHolder(srv,uav){}
		VoxelVisibility(const VoxelVisibility&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
