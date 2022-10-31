#pragma once
#include "..\Tables\VoxelOutput.h"
namespace Slots {
	struct VoxelOutput:public DataHolder<VoxelOutput, SlotID::VoxelOutput,Table::VoxelOutput,DefaultLayout::Instance2>
	{
		VoxelOutput() = default;
	};
}
