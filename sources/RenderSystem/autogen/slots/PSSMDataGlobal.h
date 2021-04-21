#pragma once
#include "..\Tables\PSSMDataGlobal.h"
namespace Slots {
	struct PSSMDataGlobal:public DataHolder<SlotID::PSSMDataGlobal,Table::PSSMDataGlobal,DefaultLayout::Instance1>
	{
		SRV srv;
		PSSMDataGlobal(): DataHolder(srv){}
		PSSMDataGlobal(const PSSMDataGlobal&other): DataHolder(srv){srv = other.srv;}
	};
}
