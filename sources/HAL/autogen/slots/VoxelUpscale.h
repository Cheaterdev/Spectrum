#pragma once
#include "..\Tables\VoxelUpscale.h"
namespace Slots {
	struct VoxelUpscale:public DataHolder<VoxelUpscale, SlotID::VoxelUpscale,Table::VoxelUpscale,DefaultLayout::Instance2>
	{
		VoxelUpscale() = default;
	};
}
