#pragma once
#include "..\Tables\DrawStencil.h"
namespace Slots {
	struct DrawStencil:public DataHolder<DrawStencil, SlotID::DrawStencil,Table::DrawStencil,DefaultLayout::Instance0>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		DrawStencil(): DataHolder(srv){}
		DrawStencil(const DrawStencil&other): DataHolder(srv){srv = other.srv;}
	};
}
