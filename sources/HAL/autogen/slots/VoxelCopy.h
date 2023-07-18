#pragma once
#include "..\Tables\VoxelCopy.h"
namespace Slots {
	struct VoxelCopy:public DataHolder<VoxelCopy, SlotID::VoxelCopy,Table::VoxelCopy,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelCopy() = default;
	};
}
