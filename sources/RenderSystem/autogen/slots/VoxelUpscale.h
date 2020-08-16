#pragma once
#include "..\Tables\VoxelUpscale.h"
namespace Slots {
	struct VoxelUpscale:public DataHolder<Table::VoxelUpscale,DefaultLayout::Instance2>
	{
		SRV srv;
		VoxelUpscale(): DataHolder(srv){}
		VoxelUpscale(const VoxelUpscale&other): DataHolder(srv){srv = other.srv;}
	};
}
