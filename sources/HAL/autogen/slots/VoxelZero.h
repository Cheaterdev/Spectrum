#pragma once
#include "..\Tables\VoxelZero.h"
namespace Slots {
	struct VoxelZero:public DataHolder<VoxelZero, SlotID::VoxelZero,Table::VoxelZero,DefaultLayout::Instance1>
	{
		VoxelZero() = default;
	};
}
