#pragma once
#include "..\Tables\RaytracingRays.h"
namespace Slots {
	struct RaytracingRays:public DataHolder<Table::RaytracingRays,DefaultLayout::Instance2>
	{
		SRV srv;
		UAV uav;
		RaytracingRays(): DataHolder(srv,uav){}
		RaytracingRays(const RaytracingRays&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
