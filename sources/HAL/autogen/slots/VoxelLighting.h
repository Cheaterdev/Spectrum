#pragma once
#include "..\Tables\VoxelLighting.h"
namespace Slots {
	struct VoxelLighting:public DataHolder<VoxelLighting, SlotID::VoxelLighting,Table::VoxelLighting,DefaultLayout::Instance1>
	{
		VoxelLighting() = default;
	};
}
