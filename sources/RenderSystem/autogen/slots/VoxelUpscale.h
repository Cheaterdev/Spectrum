#pragma once
#include "..\Tables\VoxelUpscale.h"
namespace Slots {
	struct VoxelUpscale:public DataHolder<VoxelUpscale, SlotID::VoxelUpscale,Table::VoxelUpscale,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		VoxelUpscale(): DataHolder(srv){}
		VoxelUpscale(const VoxelUpscale&other): DataHolder(srv){srv = other.srv;}
	};
}
