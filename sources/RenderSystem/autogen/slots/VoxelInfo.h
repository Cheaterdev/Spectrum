#pragma once
#include "..\Tables\VoxelInfo.h"
namespace Slots {
	struct VoxelInfo:public DataHolder<Table::VoxelInfo,DefaultLayout::Instance0>
	{
		CB cb;
		VoxelInfo(): DataHolder(cb){}
		VoxelInfo(const VoxelInfo&other): DataHolder(cb){cb = other.cb;}
	};
}
