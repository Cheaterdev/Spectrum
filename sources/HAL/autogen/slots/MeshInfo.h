#pragma once
#include "..\Tables\MeshInfo.h"
namespace Slots {
	struct MeshInfo:public DataHolder<MeshInfo, SlotID::MeshInfo,Table::MeshInfo,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MeshInfo() = default;
	};
}
