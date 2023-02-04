#pragma once
#include "..\Tables\DenoiserShadow_Fileter.h"
namespace Slots {
	struct DenoiserShadow_Fileter:public DataHolder<DenoiserShadow_Fileter, SlotID::DenoiserShadow_Fileter,Table::DenoiserShadow_Fileter,DefaultLayout::Instance0>
	{
		DenoiserShadow_Fileter() = default;
	};
}
