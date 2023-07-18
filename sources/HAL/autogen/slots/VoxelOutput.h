#pragma once
#include "..\Tables\VoxelOutput.h"
namespace Slots {
	struct VoxelOutput:public DataHolder<VoxelOutput, SlotID::VoxelOutput,Table::VoxelOutput,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelOutput() = default;
	};
}
