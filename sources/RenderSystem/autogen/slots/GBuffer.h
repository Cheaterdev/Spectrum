#pragma once
#include "..\Tables\GBuffer.h"
namespace Slots {
	struct GBuffer:public DataHolder<Table::GBuffer,DefaultLayout::Instance2>
	{
		SRV srv;
		GBuffer(): DataHolder(srv){}
		GBuffer(const GBuffer&other): DataHolder(srv){srv = other.srv;}
	};
}
