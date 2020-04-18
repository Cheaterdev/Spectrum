#pragma once
#include "..\Tables\UIRender.h"
namespace Slots {
	struct UIRender:public DataHolder<Table::UIRender,DefaultLayout::Instance0>
	{
		SRV srv;
		UIRender(): DataHolder(srv){}
	};
}
