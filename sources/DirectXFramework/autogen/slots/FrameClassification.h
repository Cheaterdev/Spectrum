#pragma once
#include "..\Tables\FrameClassification.h"
namespace Slots {
	struct FrameClassification:public DataHolder<FrameClassification, SlotID::FrameClassification,Table::FrameClassification,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID, DefaultLayout::Instance2::UAV_ID};
		FrameClassification(): DataHolder(srv,uav){}
		FrameClassification(const FrameClassification&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
