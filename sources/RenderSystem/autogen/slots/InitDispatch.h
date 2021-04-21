#pragma once
#include "..\Tables\InitDispatch.h"
namespace Slots {
	struct InitDispatch:public DataHolder<SlotID::InitDispatch,Table::InitDispatch,DefaultLayout::Instance1>
	{
		UAV uav;
		InitDispatch(): DataHolder(uav){}
		InitDispatch(const InitDispatch&other): DataHolder(uav){uav = other.uav;}
	};
}
