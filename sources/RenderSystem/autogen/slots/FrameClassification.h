#pragma once
#include "..\Tables\FrameClassification.h"
namespace Slots {
	struct FrameClassification:public DataHolder<SlotID::FrameClassification,Table::FrameClassification,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		FrameClassification(): DataHolder(srv,uav){}
		FrameClassification(const FrameClassification&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
