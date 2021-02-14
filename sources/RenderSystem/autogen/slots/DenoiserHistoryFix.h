#pragma once
#include "..\Tables\DenoiserHistoryFix.h"
namespace Slots {
	struct DenoiserHistoryFix:public DataHolder<Table::DenoiserHistoryFix,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		DenoiserHistoryFix(): DataHolder(srv,uav){}
		DenoiserHistoryFix(const DenoiserHistoryFix&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
