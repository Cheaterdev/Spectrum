#pragma once
#include "..\Tables\DenoiserShadow_Filter.h"
namespace Slots {
	struct DenoiserShadow_Filter:public DataHolder<DenoiserShadow_Filter, SlotID::DenoiserShadow_Filter,Table::DenoiserShadow_Filter,DefaultLayout::Instance0>
	{
		DenoiserShadow_Filter() = default;
	};
}
