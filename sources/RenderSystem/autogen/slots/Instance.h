#pragma once
#include "..\Tables\Instance.h"
namespace Slots {
	struct Instance:public DataHolder<Table::Instance,DefaultLayout::Instance2>
	{
		CB cb;
		Instance(): DataHolder(cb){}
		Instance(const Instance&other): DataHolder(cb){cb = other.cb;}
	};
}
