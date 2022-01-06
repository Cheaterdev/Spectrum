#pragma once
#include "..\Tables\TilingPostprocess.h"
namespace Slots {
	struct TilingPostprocess:public DataHolder<TilingPostprocess, SlotID::TilingPostprocess,Table::TilingPostprocess,FrameLayout::PassData>
	{
		TilingPostprocess() = default;
	};
}
