#pragma once
#include "..\Tables\MeshInstanceInfo.h"
namespace Slots {
	struct MeshInstanceInfo:public DataHolder<MeshInstanceInfo, SlotID::MeshInstanceInfo,Table::MeshInstanceInfo,DefaultLayout::Instance2>
	{
		MeshInstanceInfo() = default;
	};
}
