#pragma once
#include "..\Tables\EnvSource.h"
namespace Slots {
	struct EnvSource:public DataHolder<Table::EnvSource,DefaultLayout::Instance2>
	{
		SRV srv;
		EnvSource(): DataHolder(srv){}
		EnvSource(const EnvSource&other): DataHolder(srv){srv = other.srv;}
	};
}
