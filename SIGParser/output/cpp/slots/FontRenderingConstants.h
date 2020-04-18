#pragma once
#include "..\Tables\FontRenderingConstants.h"
namespace Slots {
	struct FontRenderingConstants:public DataHolder<Table::FontRenderingConstants,DefaultLayout::Instance1>
	{
		CB cb;
		FontRenderingConstants(): DataHolder(cb){}
	};
}
