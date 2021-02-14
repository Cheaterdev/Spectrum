#pragma once
#include "..\Tables\DenoiserDownsample.h"
namespace Slots {
	struct DenoiserDownsample:public DataHolder<Table::DenoiserDownsample,DefaultLayout::Instance2>
	{
		SRV srv;
		DenoiserDownsample(): DataHolder(srv){}
		DenoiserDownsample(const DenoiserDownsample&other): DataHolder(srv){srv = other.srv;}
	};
}
