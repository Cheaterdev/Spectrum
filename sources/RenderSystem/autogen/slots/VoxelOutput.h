#pragma once
#include "..\Tables\VoxelOutput.h"
namespace Slots {
	struct VoxelOutput:public DataHolder<SlotID::VoxelOutput,Table::VoxelOutput,DefaultLayout::Instance2>
	{
		UAV uav;
		VoxelOutput(): DataHolder(uav){}
		VoxelOutput(const VoxelOutput&other): DataHolder(uav){uav = other.uav;}
	};
}
