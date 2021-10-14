#pragma once
#include "..\Tables\VoxelScreen.h"
namespace Slots {
	struct VoxelScreen:public DataHolder<VoxelScreen, SlotID::VoxelScreen,Table::VoxelScreen,DefaultLayout::Instance1>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID};
		VoxelScreen(): DataHolder(srv){}
		VoxelScreen(const VoxelScreen&other): DataHolder(srv){srv = other.srv;}
	};
}
