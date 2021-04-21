#pragma once
#include "..\Tables\DenoiserDownsample.h"
namespace Slots {
	struct DenoiserDownsample:public DataHolder<SlotID::DenoiserDownsample,Table::DenoiserDownsample,DefaultLayout::Instance2>
	{
		SRV srv;
		DenoiserDownsample(): DataHolder(srv){}
		DenoiserDownsample(const DenoiserDownsample&other): DataHolder(srv){srv = other.srv;}
	};
}
