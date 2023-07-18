#pragma once
#include "..\Tables\VoxelBlur.h"
namespace Slots {
	struct VoxelBlur:public DataHolder<VoxelBlur, SlotID::VoxelBlur,Table::VoxelBlur,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelBlur() = default;
	};
}
