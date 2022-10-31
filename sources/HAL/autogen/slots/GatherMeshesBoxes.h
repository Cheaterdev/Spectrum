#pragma once
#include "..\Tables\GatherMeshesBoxes.h"
namespace Slots {
	struct GatherMeshesBoxes:public DataHolder<GatherMeshesBoxes, SlotID::GatherMeshesBoxes,Table::GatherMeshesBoxes,DefaultLayout::Instance1>
	{
		GatherMeshesBoxes() = default;
	};
}
