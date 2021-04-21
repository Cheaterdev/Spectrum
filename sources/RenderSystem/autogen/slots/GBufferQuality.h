#pragma once
#include "..\Tables\GBufferQuality.h"
namespace Slots {
	struct GBufferQuality:public DataHolder<SlotID::GBufferQuality,Table::GBufferQuality,DefaultLayout::Instance2>
	{
		SRV srv;
		GBufferQuality(): DataHolder(srv){}
		GBufferQuality(const GBufferQuality&other): DataHolder(srv){srv = other.srv;}
	};
}
