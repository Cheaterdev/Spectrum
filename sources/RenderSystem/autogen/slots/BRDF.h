#pragma once
#include "..\Tables\BRDF.h"
namespace Slots {
	struct BRDF:public DataHolder<BRDF, SlotID::BRDF,Table::BRDF,DefaultLayout::Instance0>
	{
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::UAV_ID};
		BRDF(): DataHolder(uav){}
		BRDF(const BRDF&other): DataHolder(uav){uav = other.uav;}
	};
}
