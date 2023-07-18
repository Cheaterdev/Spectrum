#pragma once
#include "..\Tables\GatherMeshesBoxes.h"
namespace Slots {
	struct GatherMeshesBoxes:public DataHolder<GatherMeshesBoxes, SlotID::GatherMeshesBoxes,Table::GatherMeshesBoxes,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherMeshesBoxes() = default;
	};
}
