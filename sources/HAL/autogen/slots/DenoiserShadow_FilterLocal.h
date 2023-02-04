#pragma once
#include "..\Tables\DenoiserShadow_FilterLocal.h"
namespace Slots {
	struct DenoiserShadow_FilterLocal:public DataHolder<DenoiserShadow_FilterLocal, SlotID::DenoiserShadow_FilterLocal,Table::DenoiserShadow_FilterLocal,DefaultLayout::Instance1>
	{
		DenoiserShadow_FilterLocal() = default;
	};
}
