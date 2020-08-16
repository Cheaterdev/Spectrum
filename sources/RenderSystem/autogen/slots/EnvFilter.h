#pragma once
#include "..\Tables\EnvFilter.h"
namespace Slots {
	struct EnvFilter:public DataHolder<Table::EnvFilter,DefaultLayout::Instance1>
	{
		CB cb;
		EnvFilter(): DataHolder(cb){}
		EnvFilter(const EnvFilter&other): DataHolder(cb){cb = other.cb;}
	};
}
