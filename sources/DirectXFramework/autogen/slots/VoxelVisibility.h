#pragma once
#include "..\Tables\VoxelVisibility.h"
namespace Slots {
	struct VoxelVisibility:public DataHolder<VoxelVisibility, SlotID::VoxelVisibility,Table::VoxelVisibility,DefaultLayout::Instance1>
	{
		VoxelVisibility() = default;
	};
}
