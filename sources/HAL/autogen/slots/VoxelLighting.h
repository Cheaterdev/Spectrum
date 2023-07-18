#pragma once
#include "..\Tables\VoxelLighting.h"
namespace Slots {
	struct VoxelLighting:public DataHolder<VoxelLighting, SlotID::VoxelLighting,Table::VoxelLighting,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelLighting() = default;
	};
}
