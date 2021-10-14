#pragma once
#include "..\Tables\DrawBoxes.h"
namespace Slots {
	struct DrawBoxes:public DataHolder<DrawBoxes, SlotID::DrawBoxes,Table::DrawBoxes,DefaultLayout::Instance1>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		DrawBoxes(): DataHolder(srv,uav){}
		DrawBoxes(const DrawBoxes&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
