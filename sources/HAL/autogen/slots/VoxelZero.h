#pragma once
#include "..\Tables\VoxelZero.h"
namespace Slots {
	struct VoxelZero:public DataHolder<VoxelZero, SlotID::VoxelZero,Table::VoxelZero,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelZero() = default;
	};
}
