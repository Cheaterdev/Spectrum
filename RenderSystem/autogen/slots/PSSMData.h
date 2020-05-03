#pragma once
#include "..\Tables\PSSMData.h"
namespace Slots {
	struct PSSMData:public DataHolder<Table::PSSMData,DefaultLayout::Instance1>
	{
		SRV srv;
		PSSMData(): DataHolder(srv){}
		PSSMData(const PSSMData&other): DataHolder(srv){srv = other.srv;}
	};
}
