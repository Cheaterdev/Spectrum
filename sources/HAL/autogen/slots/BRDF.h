#pragma once
#include "..\Tables\BRDF.h"
namespace Slots {
	struct BRDF:public DataHolder<BRDF, SlotID::BRDF,Table::BRDF,DefaultLayout::Instance0>
	{
		BRDF() = default;
	};
}
