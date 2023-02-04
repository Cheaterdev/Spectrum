#pragma once
#include "..\Tables\DenoiserShadow_Prepare.h"
namespace Slots {
	struct DenoiserShadow_Prepare:public DataHolder<DenoiserShadow_Prepare, SlotID::DenoiserShadow_Prepare,Table::DenoiserShadow_Prepare,DefaultLayout::Instance0>
	{
		DenoiserShadow_Prepare() = default;
	};
}
