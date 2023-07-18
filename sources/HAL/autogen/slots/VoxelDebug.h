#pragma once
#include "..\Tables\VoxelDebug.h"
namespace Slots {
	struct VoxelDebug:public DataHolder<VoxelDebug, SlotID::VoxelDebug,Table::VoxelDebug,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelDebug() = default;
	};
}
