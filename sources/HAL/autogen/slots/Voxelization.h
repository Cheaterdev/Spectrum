#pragma once
#include "..\Tables\Voxelization.h"
namespace Slots {
	struct Voxelization:public DataHolder<Voxelization, SlotID::Voxelization,Table::Voxelization,DefaultLayout::Instance2>
	{
		Voxelization() = default;
	};
}
