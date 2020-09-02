#pragma once
#include "..\Tables\RaytracingRays.h"
namespace Slots {
	struct RaytracingRays:public DataHolder<Table::RaytracingRays,DefaultLayout::Instance2>
	{
		CB cb;
		SRV srv;
		UAV uav;
		RaytracingRays(): DataHolder(cb,srv,uav){}
		RaytracingRays(const RaytracingRays&other): DataHolder(cb,srv,uav){cb = other.cb;srv = other.srv;uav = other.uav;}
	};
}
