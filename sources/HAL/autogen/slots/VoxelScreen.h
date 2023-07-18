#pragma once
#include "..\Tables\VoxelScreen.h"
namespace Slots {
	struct VoxelScreen:public DataHolder<VoxelScreen, SlotID::VoxelScreen,Table::VoxelScreen,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelScreen() = default;
	};
}
