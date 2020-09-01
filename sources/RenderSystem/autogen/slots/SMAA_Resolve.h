#pragma once
#include "..\Tables\SMAA_Resolve.h"
namespace Slots {
	struct SMAA_Resolve:public DataHolder<Table::SMAA_Resolve,DefaultLayout::Instance0>
	{
		SRV srv;
		SMAA_Resolve(): DataHolder(srv){}
		SMAA_Resolve(const SMAA_Resolve&other): DataHolder(srv){srv = other.srv;}
	};
}
