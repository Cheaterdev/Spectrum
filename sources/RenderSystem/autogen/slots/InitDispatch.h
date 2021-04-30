#pragma once
#include "..\Tables\InitDispatch.h"
namespace Slots {
	struct InitDispatch:public DataHolder<InitDispatch, SlotID::InitDispatch,Table::InitDispatch,DefaultLayout::Instance1>
	{
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::UAV_ID};
		InitDispatch(): DataHolder(uav){}
		InitDispatch(const InitDispatch&other): DataHolder(uav){uav = other.uav;}
	};
}
