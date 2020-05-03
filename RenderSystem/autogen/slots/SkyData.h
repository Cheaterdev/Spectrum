#pragma once
#include "..\Tables\SkyData.h"
namespace Slots {
	struct SkyData:public DataHolder<Table::SkyData,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		SkyData(): DataHolder(cb,srv){}
		SkyData(const SkyData&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
