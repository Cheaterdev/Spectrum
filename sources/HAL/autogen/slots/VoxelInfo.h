#pragma once
#include "..\Tables\VoxelInfo.h"
namespace Slots {
	struct VoxelInfo:public DataHolder<VoxelInfo, SlotID::VoxelInfo,Table::VoxelInfo,DefaultLayout::Instance0>
	{
		VoxelInfo() = default;
	};
}
