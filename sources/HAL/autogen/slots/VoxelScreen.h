#pragma once
#include "..\Tables\VoxelScreen.h"
namespace Slots {
	struct VoxelScreen:public DataHolder<VoxelScreen, SlotID::VoxelScreen,Table::VoxelScreen,DefaultLayout::Instance1>
	{
		VoxelScreen() = default;
	};
}
