#pragma once
#include "..\Tables\DenoiserDownsample.h"
namespace Slots {
	struct DenoiserDownsample:public DataHolder<DenoiserDownsample, SlotID::DenoiserDownsample,Table::DenoiserDownsample,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		DenoiserDownsample(): DataHolder(srv){}
		DenoiserDownsample(const DenoiserDownsample&other): DataHolder(srv){srv = other.srv;}
	};
}
