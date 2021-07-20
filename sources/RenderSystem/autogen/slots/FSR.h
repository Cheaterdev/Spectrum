#pragma once
#include "..\Tables\FSR.h"
namespace Slots {
	struct FSR:public DataHolder<FSR, SlotID::FSR,Table::FSR,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID, DefaultLayout::Instance0::SRV_ID, DefaultLayout::Instance0::UAV_ID};
		FSR(): DataHolder(cb,srv,uav){}
		FSR(const FSR&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
