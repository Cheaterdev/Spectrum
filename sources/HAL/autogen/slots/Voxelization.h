#pragma once
#include "..\Tables\Voxelization.h"
namespace Slots {
	struct Voxelization:public DataHolder<Voxelization, SlotID::Voxelization,Table::Voxelization,DefaultLayout::Instance3>
	{
		Voxelization() = default;
	};
}
