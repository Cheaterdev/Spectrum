#pragma once
#include "..\Tables\VoxelBlur.h"
namespace Slots {
	struct VoxelBlur:public DataHolder<VoxelBlur, SlotID::VoxelBlur,Table::VoxelBlur,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID, DefaultLayout::Instance2::UAV_ID};
		VoxelBlur(): DataHolder(srv,uav){}
		VoxelBlur(const VoxelBlur&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
