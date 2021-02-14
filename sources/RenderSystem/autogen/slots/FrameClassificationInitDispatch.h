#pragma once
#include "..\Tables\FrameClassificationInitDispatch.h"
namespace Slots {
	struct FrameClassificationInitDispatch:public DataHolder<Table::FrameClassificationInitDispatch,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		FrameClassificationInitDispatch(): DataHolder(srv,uav){}
		FrameClassificationInitDispatch(const FrameClassificationInitDispatch&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
