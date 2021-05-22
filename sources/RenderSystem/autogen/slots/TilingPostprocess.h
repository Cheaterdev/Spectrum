#pragma once
#include "..\Tables\TilingPostprocess.h"
namespace Slots {
	struct TilingPostprocess:public DataHolder<TilingPostprocess, SlotID::TilingPostprocess,Table::TilingPostprocess,FrameLayout::PassData>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {FrameLayout::PassData::SRV_ID};
		TilingPostprocess(): DataHolder(srv){}
		TilingPostprocess(const TilingPostprocess&other): DataHolder(srv){srv = other.srv;}
	};
}
