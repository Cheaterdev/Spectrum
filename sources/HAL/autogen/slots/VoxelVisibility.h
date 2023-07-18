#pragma once
#include "..\Tables\VoxelVisibility.h"
namespace Slots {
	struct VoxelVisibility:public DataHolder<VoxelVisibility, SlotID::VoxelVisibility,Table::VoxelVisibility,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelVisibility() = default;
	};
}
