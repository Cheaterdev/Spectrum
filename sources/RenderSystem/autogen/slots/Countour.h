#pragma once
#include "..\Tables\Countour.h"
namespace Slots {
	struct Countour:public DataHolder<Table::Countour,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		Countour(): DataHolder(cb,srv){}
		Countour(const Countour&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
