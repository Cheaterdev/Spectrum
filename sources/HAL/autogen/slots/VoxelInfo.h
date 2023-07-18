#pragma once
#include "..\Tables\VoxelInfo.h"
namespace Slots {
	struct VoxelInfo:public DataHolder<VoxelInfo, SlotID::VoxelInfo,Table::VoxelInfo,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelInfo() = default;
	};
}
