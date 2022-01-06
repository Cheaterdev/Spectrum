#pragma once
#include "..\Tables\MeshInfo.h"
namespace Slots {
	struct MeshInfo:public DataHolder<MeshInfo, SlotID::MeshInfo,Table::MeshInfo,DefaultLayout::Instance1>
	{
		MeshInfo() = default;
	};
}
