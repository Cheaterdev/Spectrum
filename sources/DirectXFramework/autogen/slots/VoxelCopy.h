#pragma once
#include "..\Tables\VoxelCopy.h"
namespace Slots {
	struct VoxelCopy:public DataHolder<VoxelCopy, SlotID::VoxelCopy,Table::VoxelCopy,DefaultLayout::Instance1>
	{
		VoxelCopy() = default;
	};
}
