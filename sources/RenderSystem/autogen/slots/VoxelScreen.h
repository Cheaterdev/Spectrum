#pragma once
#include "..\Tables\VoxelScreen.h"
namespace Slots {
	struct VoxelScreen:public DataHolder<SlotID::VoxelScreen,Table::VoxelScreen,DefaultLayout::Instance1>
	{
		SRV srv;
		VoxelScreen(): DataHolder(srv){}
		VoxelScreen(const VoxelScreen&other): DataHolder(srv){srv = other.srv;}
	};
}
