#pragma once
#include "..\Tables\FrameClassificationInitDispatch.h"
namespace Slots {
	struct FrameClassificationInitDispatch:public DataHolder<FrameClassificationInitDispatch, SlotID::FrameClassificationInitDispatch,Table::FrameClassificationInitDispatch,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID, DefaultLayout::Instance2::UAV_ID};
		FrameClassificationInitDispatch(): DataHolder(srv,uav){}
		FrameClassificationInitDispatch(const FrameClassificationInitDispatch&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
