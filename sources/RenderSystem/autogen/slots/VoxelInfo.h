#pragma once
#include "..\Tables\VoxelInfo.h"
namespace Slots {
	struct VoxelInfo:public DataHolder<VoxelInfo, SlotID::VoxelInfo,Table::VoxelInfo,DefaultLayout::Instance0>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID};
		VoxelInfo(): DataHolder(cb){}
		VoxelInfo(const VoxelInfo&other): DataHolder(cb){cb = other.cb;}
	};
}
