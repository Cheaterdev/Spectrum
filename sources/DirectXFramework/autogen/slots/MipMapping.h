#pragma once
#include "..\Tables\MipMapping.h"
namespace Slots {
	struct MipMapping:public DataHolder<MipMapping, SlotID::MipMapping,Table::MipMapping,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID, DefaultLayout::Instance0::SRV_ID, DefaultLayout::Instance0::UAV_ID};
		MipMapping(): DataHolder(cb,srv,uav){}
		MipMapping(const MipMapping&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
