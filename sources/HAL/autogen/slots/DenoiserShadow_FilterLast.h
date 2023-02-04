#pragma once
#include "..\Tables\DenoiserShadow_FilterLast.h"
namespace Slots {
	struct DenoiserShadow_FilterLast:public DataHolder<DenoiserShadow_FilterLast, SlotID::DenoiserShadow_FilterLast,Table::DenoiserShadow_FilterLast,DefaultLayout::Instance1>
	{
		DenoiserShadow_FilterLast() = default;
	};
}
