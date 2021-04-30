#pragma once
#include "..\Tables\SkyData.h"
namespace Slots {
	struct SkyData:public DataHolder<SkyData, SlotID::SkyData,Table::SkyData,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID, DefaultLayout::Instance0::SRV_ID};
		SkyData(): DataHolder(cb,srv){}
		SkyData(const SkyData&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
