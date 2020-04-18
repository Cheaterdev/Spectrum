#pragma once
#include "..\Tables\PSSMLighting.h"
namespace Slots {
	struct PSSMLighting:public DataHolder<Table::PSSMLighting,DefaultLayout::Instance2>
	{
		SRV srv;
		PSSMLighting(): DataHolder(srv){}
	};
}
