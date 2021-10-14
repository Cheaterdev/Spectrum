#pragma once
#include "..\Tables\RaytracingRays.h"
namespace Slots {
	struct RaytracingRays:public DataHolder<RaytracingRays, SlotID::RaytracingRays,Table::RaytracingRays,DefaultLayout::Instance2>
	{
		CB cb;
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::CB_ID, DefaultLayout::Instance2::SRV_ID, DefaultLayout::Instance2::UAV_ID};
		RaytracingRays(): DataHolder(cb,srv,uav){}
		RaytracingRays(const RaytracingRays&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
