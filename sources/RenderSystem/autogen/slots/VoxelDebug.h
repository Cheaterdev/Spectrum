#pragma once
#include "..\Tables\VoxelDebug.h"
namespace Slots {
	struct VoxelDebug:public DataHolder<VoxelDebug, SlotID::VoxelDebug,Table::VoxelDebug,DefaultLayout::Instance1>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID};
		VoxelDebug(): DataHolder(srv){}
		VoxelDebug(const VoxelDebug&other): DataHolder(srv){srv = other.srv;}
	};
}
