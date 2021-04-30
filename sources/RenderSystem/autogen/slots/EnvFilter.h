#pragma once
#include "..\Tables\EnvFilter.h"
namespace Slots {
	struct EnvFilter:public DataHolder<EnvFilter, SlotID::EnvFilter,Table::EnvFilter,DefaultLayout::Instance1>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID};
		EnvFilter(): DataHolder(cb){}
		EnvFilter(const EnvFilter&other): DataHolder(cb){cb = other.cb;}
	};
}
