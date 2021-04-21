#pragma once
#include "..\Tables\BRDF.h"
namespace Slots {
	struct BRDF:public DataHolder<SlotID::BRDF,Table::BRDF,DefaultLayout::Instance0>
	{
		UAV uav;
		BRDF(): DataHolder(uav){}
		BRDF(const BRDF&other): DataHolder(uav){uav = other.uav;}
	};
}
