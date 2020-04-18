#pragma once
#include "..\Tables\ColorRect.h"
namespace Slots {
	struct ColorRect:public DataHolder<Table::ColorRect,DefaultLayout::Instance0>
	{
		CB cb;
		ColorRect(): DataHolder(cb){}
	};
}
