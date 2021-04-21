#pragma once
#include "..\Tables\DrawBoxes.h"
namespace Slots {
	struct DrawBoxes:public DataHolder<SlotID::DrawBoxes,Table::DrawBoxes,DefaultLayout::Instance1>
	{
		SRV srv;
		UAV uav;
		DrawBoxes(): DataHolder(srv,uav){}
		DrawBoxes(const DrawBoxes&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
