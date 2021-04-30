#pragma once
#include "..\Tables\Voxelization.h"
namespace Slots {
	struct Voxelization:public DataHolder<Voxelization, SlotID::Voxelization,Table::Voxelization,DefaultLayout::Instance2>
	{
		CB cb;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::CB_ID, DefaultLayout::Instance2::UAV_ID};
		Voxelization(): DataHolder(cb,uav){}
		Voxelization(const Voxelization&other): DataHolder(cb,uav){cb = other.cb;uav = other.uav;}
	};
}
