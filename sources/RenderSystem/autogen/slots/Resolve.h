#pragma once
#include "..\Tables\Resolve.h"
namespace Slots {
	struct Resolve:public DataHolder<Table::Resolve,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		Resolve(): DataHolder(cb,srv){}
		Resolve(const Resolve&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
