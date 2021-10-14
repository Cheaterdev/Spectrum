#pragma once
#include "..\Tables\GBufferQuality.h"
namespace Slots {
	struct GBufferQuality:public DataHolder<GBufferQuality, SlotID::GBufferQuality,Table::GBufferQuality,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		GBufferQuality(): DataHolder(srv){}
		GBufferQuality(const GBufferQuality&other): DataHolder(srv){srv = other.srv;}
	};
}
