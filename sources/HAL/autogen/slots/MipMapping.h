#pragma once
#include "..\Tables\MipMapping.h"
namespace Slots {
	struct MipMapping:public DataHolder<MipMapping, SlotID::MipMapping,Table::MipMapping,DefaultLayout::Instance0>
	{
		MipMapping() = default;
	};
}
