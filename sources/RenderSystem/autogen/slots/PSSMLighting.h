#pragma once
#include "..\Tables\PSSMLighting.h"
namespace Slots {
	struct PSSMLighting:public DataHolder<PSSMLighting, SlotID::PSSMLighting,Table::PSSMLighting,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		PSSMLighting(): DataHolder(srv){}
		PSSMLighting(const PSSMLighting&other): DataHolder(srv){srv = other.srv;}
	};
}
