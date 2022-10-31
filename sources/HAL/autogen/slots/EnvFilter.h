#pragma once
#include "..\Tables\EnvFilter.h"
namespace Slots {
	struct EnvFilter:public DataHolder<EnvFilter, SlotID::EnvFilter,Table::EnvFilter,DefaultLayout::Instance1>
	{
		EnvFilter() = default;
	};
}
