#pragma once
#include "..\Tables\GBuffer.h"
namespace Slots {
	struct GBuffer:public DataHolder<GBuffer, SlotID::GBuffer,Table::GBuffer,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		GBuffer(): DataHolder(srv){}
		GBuffer(const GBuffer&other): DataHolder(srv){srv = other.srv;}
	};
}
