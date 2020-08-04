#pragma once
#include "..\Tables\GBufferDownsample.h"
namespace Slots {
	struct GBufferDownsample:public DataHolder<Table::GBufferDownsample,DefaultLayout::Instance2>
	{
		SRV srv;
		GBufferDownsample(): DataHolder(srv){}
		GBufferDownsample(const GBufferDownsample&other): DataHolder(srv){srv = other.srv;}
	};
}
