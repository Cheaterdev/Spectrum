#pragma once
#include "..\Tables\NinePatch.h"
namespace Slots {
	struct NinePatch:public DataHolder<Table::NinePatch,DefaultLayout::Instance0>
	{
		SRV srv;
		Render::Bindless bindless;
		NinePatch(): DataHolder(srv,bindless){}
	};
}
