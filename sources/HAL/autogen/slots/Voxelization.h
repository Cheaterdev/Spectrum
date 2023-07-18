#pragma once
#include "..\Tables\Voxelization.h"
namespace Slots {
	struct Voxelization:public DataHolder<Voxelization, SlotID::Voxelization,Table::Voxelization,DefaultLayout::Instance3>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Voxelization() = default;
	};
}
