#pragma once
#include "..\Tables\DenoiserHistoryFix.h"
namespace Slots {
	struct DenoiserHistoryFix:public DataHolder<DenoiserHistoryFix, SlotID::DenoiserHistoryFix,Table::DenoiserHistoryFix,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID, DefaultLayout::Instance2::UAV_ID};
		DenoiserHistoryFix(): DataHolder(srv,uav){}
		DenoiserHistoryFix(const DenoiserHistoryFix&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
