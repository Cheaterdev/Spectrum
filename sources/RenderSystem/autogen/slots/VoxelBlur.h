#pragma once
#include "..\Tables\VoxelBlur.h"
namespace Slots {
	struct VoxelBlur:public DataHolder<Table::VoxelBlur,DefaultLayout::Instance2>
	{
		SRV srv;
		VoxelBlur(): DataHolder(srv){}
		VoxelBlur(const VoxelBlur&other): DataHolder(srv){srv = other.srv;}
	};
}
