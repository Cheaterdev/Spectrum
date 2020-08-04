#pragma once
#include "..\Tables\VoxelDebug.h"
namespace Slots {
	struct VoxelDebug:public DataHolder<Table::VoxelDebug,DefaultLayout::Instance1>
	{
		SRV srv;
		VoxelDebug(): DataHolder(srv){}
		VoxelDebug(const VoxelDebug&other): DataHolder(srv){srv = other.srv;}
	};
}
