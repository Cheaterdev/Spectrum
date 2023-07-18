#pragma once
#include "..\Tables\VoxelMipMap.h"
namespace Slots {
	struct VoxelMipMap:public DataHolder<VoxelMipMap, SlotID::VoxelMipMap,Table::VoxelMipMap,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelMipMap() = default;
	};
}
