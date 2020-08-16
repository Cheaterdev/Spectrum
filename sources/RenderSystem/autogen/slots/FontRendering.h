#pragma once
#include "..\Tables\FontRendering.h"
namespace Slots {
	struct FontRendering:public DataHolder<Table::FontRendering,DefaultLayout::Instance0>
	{
		SRV srv;
		FontRendering(): DataHolder(srv){}
		FontRendering(const FontRendering&other): DataHolder(srv){srv = other.srv;}
	};
}
