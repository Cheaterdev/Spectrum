#pragma once
#include "..\Tables\GBufferDownsample.h"
namespace Slots {
	struct GBufferDownsample:public DataHolder<GBufferDownsample, SlotID::GBufferDownsample,Table::GBufferDownsample,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		GBufferDownsample(): DataHolder(srv){}
		GBufferDownsample(const GBufferDownsample&other): DataHolder(srv){srv = other.srv;}
	};
}
