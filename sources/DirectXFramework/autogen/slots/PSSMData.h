#pragma once
#include "..\Tables\PSSMData.h"
namespace Slots {
	struct PSSMData:public DataHolder<PSSMData, SlotID::PSSMData,Table::PSSMData,DefaultLayout::Instance1>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID};
		PSSMData(): DataHolder(srv){}
		PSSMData(const PSSMData&other): DataHolder(srv){srv = other.srv;}
	};
}
