#pragma once
#include "..\Tables\DrawStencil.h"
namespace Slots {
	struct DrawStencil:public DataHolder<Table::DrawStencil,DefaultLayout::Instance0>
	{
		SRV srv;
		DrawStencil(): DataHolder(srv){}
		DrawStencil(const DrawStencil&other): DataHolder(srv){srv = other.srv;}
	};
}
