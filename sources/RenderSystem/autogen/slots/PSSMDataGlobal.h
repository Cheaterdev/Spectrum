#pragma once
#include "..\Tables\PSSMDataGlobal.h"
namespace Slots {
	struct PSSMDataGlobal:public DataHolder<PSSMDataGlobal, SlotID::PSSMDataGlobal,Table::PSSMDataGlobal,DefaultLayout::Instance1>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID};
		PSSMDataGlobal(): DataHolder(srv){}
		PSSMDataGlobal(const PSSMDataGlobal&other): DataHolder(srv){srv = other.srv;}
	};
}
