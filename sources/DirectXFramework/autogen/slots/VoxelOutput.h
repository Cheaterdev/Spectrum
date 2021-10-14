#pragma once
#include "..\Tables\VoxelOutput.h"
namespace Slots {
	struct VoxelOutput:public DataHolder<VoxelOutput, SlotID::VoxelOutput,Table::VoxelOutput,DefaultLayout::Instance2>
	{
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::UAV_ID};
		VoxelOutput(): DataHolder(uav){}
		VoxelOutput(const VoxelOutput&other): DataHolder(uav){uav = other.uav;}
	};
}
