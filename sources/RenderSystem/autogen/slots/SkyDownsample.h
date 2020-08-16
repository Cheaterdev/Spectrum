#pragma once
#include "..\Tables\SkyDownsample.h"
namespace Slots {
	struct SkyDownsample:public DataHolder<Table::SkyDownsample,DefaultLayout::Instance2>
	{
		CB cb;
		SRV srv;
		SkyDownsample(): DataHolder(cb,srv){}
		SkyDownsample(const SkyDownsample&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
