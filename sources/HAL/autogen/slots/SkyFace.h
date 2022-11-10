#pragma once
#include "..\Tables\SkyFace.h"
namespace Slots {
	struct SkyFace:public DataHolder<SkyFace, SlotID::SkyFace,Table::SkyFace,DefaultLayout::Instance1>
	{
		SkyFace() = default;
	};
}
