#pragma once
#include "..\Tables\Color.h"
namespace Slots {
	struct Color:public DataHolder<Table::Color,DefaultLayout::Instance0>
	{
		CB cb;
		Color(): DataHolder(cb){}
		Color(const Color&other): DataHolder(cb){cb = other.cb;}
	};
}
