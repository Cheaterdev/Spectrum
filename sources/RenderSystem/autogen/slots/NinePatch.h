#pragma once
#include "..\Tables\NinePatch.h"
namespace Slots {
	struct NinePatch:public DataHolder<Table::NinePatch,DefaultLayout::Instance0>
	{
		SRV srv;
		DX12::Bindless bindless;
		NinePatch(): DataHolder(srv,bindless){}
		NinePatch(const NinePatch&other): DataHolder(srv,bindless){srv = other.srv;}
	};
}
